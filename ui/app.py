"""Local UI for inspecting GenChess training data and playing its best chromosome.

This is deliberately separate from the C++ engine and GA trainer.  It only reads
``ga/runs`` and applies the selected chromosome's five weights in the web game's
small search adapter.
"""

from __future__ import annotations

import csv
import json
import math
from dataclasses import dataclass
from pathlib import Path
from typing import Literal

import chess
from fastapi import FastAPI, HTTPException
from fastapi.responses import FileResponse
from fastapi.staticfiles import StaticFiles
from pydantic import BaseModel


ROOT = Path(__file__).resolve().parents[1]
RUNS_DIR = ROOT / "ga" / "runs"
WEIGHT_NAMES = ("material", "pawn_structure", "mobility", "king_safety", "controlled_squares")
PIECE_VALUES = {
    chess.PAWN: 1.0,
    chess.KNIGHT: 3.0,
    chess.BISHOP: 3.0,
    chess.ROOK: 5.0,
    chess.QUEEN: 9.0,
}


@dataclass(frozen=True)
class Champion:
    run: str
    generation: int
    candidate_id: int
    elo: float
    weights: tuple[float, float, float, float, float]


def find_champion() -> Champion:
    """Return the strongest logged candidate across all completed training runs."""
    records: list[Champion] = []
    for path in RUNS_DIR.glob("*/generations.csv"):
        with path.open(newline="", encoding="utf-8") as file:
            for row in csv.DictReader(file):
                records.append(Champion(
                    run=path.parent.name,
                    generation=int(row["generation"]),
                    candidate_id=int(row["candidate_id"]),
                    elo=float(row["elo"]),
                    weights=tuple(float(value) for value in json.loads(row["weights"])),
                ))
    if not records:
        raise HTTPException(404, "No generations.csv found. Run the GA trainer first.")
    return max(records, key=lambda candidate: candidate.elo)


def generation_leaders(run: str) -> list[dict[str, float | int | str]]:
    """Provide the selected run's best Elo in each generation for the SVG chart."""
    rows: list[dict[str, float | int | str]] = []
    path = RUNS_DIR / run / "generations.csv"
    leaders: dict[int, dict[str, str]] = {}
    with path.open(newline="", encoding="utf-8") as file:
        for row in csv.DictReader(file):
            generation = int(row["generation"])
            if generation not in leaders or float(row["elo"]) > float(leaders[generation]["elo"]):
                leaders[generation] = row
    rows.extend({"run": run, "generation": generation, "elo": float(row["elo"])}
                for generation, row in leaders.items())
    return sorted(rows, key=lambda row: int(row["generation"]))


def pawn_structure(board: chess.Board, colour: chess.Color) -> float:
    pawns = board.pieces(chess.PAWN, colour)
    score = 0.0
    for file_index in range(8):
        on_file = [square for square in pawns if chess.square_file(square) == file_index]
        if len(on_file) > 1:
            score -= len(on_file) - 1
        if on_file and not any(abs(chess.square_file(square) - file_index) == 1 for square in pawns):
            score -= len(on_file)
    enemy_pawns = board.pieces(chess.PAWN, not colour)
    for square in pawns:
        file_index, rank_index = chess.square_file(square), chess.square_rank(square)
        direction = 1 if colour == chess.WHITE else -1
        if not any(abs(chess.square_file(enemy) - file_index) <= 1 and
                   (chess.square_rank(enemy) - rank_index) * direction > 0 for enemy in enemy_pawns):
            score += 1.0
    return score


def controlled_squares(board: chess.Board, colour: chess.Color) -> set[chess.Square]:
    return {square for square in chess.SQUARES if board.attackers(colour, square)}


def side_mobility(board: chess.Board, colour: chess.Color) -> int:
    copied = board.copy(stack=False)
    copied.turn = colour
    return copied.legal_moves.count()


def king_safety(board: chess.Board, colour: chess.Color) -> float:
    king = board.king(colour)
    if king is None:
        return -100.0
    rank, file_index = chess.square_rank(king), chess.square_file(king)
    forward = 1 if colour == chess.WHITE else -1
    shield = sum(
        1 for target_file in range(max(0, file_index - 1), min(8, file_index + 2))
        for target_rank in [rank + forward]
        if 0 <= target_rank < 8 and board.piece_at(chess.square(target_file, target_rank)) == chess.Piece(chess.PAWN, colour)
    )
    danger = sum(1 for square in chess.SquareSet(chess.BB_KING_ATTACKS[king]) if board.is_attacked_by(not colour, square))
    return shield - danger


def evaluate(board: chess.Board, weights: tuple[float, ...]) -> float:
    material = sum(len(board.pieces(piece, chess.WHITE)) * value - len(board.pieces(piece, chess.BLACK)) * value
                   for piece, value in PIECE_VALUES.items())
    pawn = pawn_structure(board, chess.WHITE) - pawn_structure(board, chess.BLACK)
    mobility = side_mobility(board, chess.WHITE) - side_mobility(board, chess.BLACK)
    safety = king_safety(board, chess.WHITE) - king_safety(board, chess.BLACK)
    control = len(controlled_squares(board, chess.WHITE)) - len(controlled_squares(board, chess.BLACK))
    return sum(weight * feature for weight, feature in zip(weights, (material, pawn, mobility, safety, control)))


def search(board: chess.Board, weights: tuple[float, ...], depth: int, alpha: float, beta: float) -> float:
    if board.is_checkmate():
        return -10_000 - depth
    if board.is_game_over(claim_draw=True):
        return 0.0
    if depth == 0:
        score = evaluate(board, weights)
        return score if board.turn == chess.WHITE else -score
    value = -math.inf
    for move in board.legal_moves:
        board.push(move)
        value = max(value, -search(board, weights, depth - 1, -beta, -alpha))
        board.pop()
        alpha = max(alpha, value)
        if alpha >= beta:
            break
    return value


def choose_move(board: chess.Board, weights: tuple[float, ...]) -> chess.Move:
    best_move, best_score = None, -math.inf
    for move in board.legal_moves:
        board.push(move)
        score = -search(board, weights, depth=2, alpha=-math.inf, beta=math.inf)
        board.pop()
        if score > best_score:
            best_move, best_score = move, score
    if best_move is None:
        raise HTTPException(409, "The game has ended.")
    return best_move


class NewGame(BaseModel):
    human_colour: Literal["white", "black"] = "white"


class PlayerMove(BaseModel):
    move: str


@dataclass
class Game:
    board: chess.Board
    human_colour: chess.Color
    moves: list[str]
    resigned_by: chess.Color | None = None


game: Game | None = None
app = FastAPI(title="GenChess UI")
app.mount("/static", StaticFiles(directory=Path(__file__).parent / "static"), name="static")


def game_payload() -> dict:
    if game is None:
        raise HTTPException(409, "Start a new game first.")
    champion = find_champion()
    return {
        "fen": game.board.fen(),
        "turn": "white" if game.board.turn == chess.WHITE else "black",
        "human_colour": "white" if game.human_colour == chess.WHITE else "black",
        "moves": game.moves,
        "outcome": (
            ("Black wins by resignation" if game.resigned_by == chess.WHITE else "White wins by resignation")
            if game.resigned_by is not None
            else game.board.outcome(claim_draw=True).result() if game.board.is_game_over(claim_draw=True) else None
        ),
        "champion": champion.__dict__ | {"weights": list(champion.weights)},
    }


def let_bot_play() -> None:
    assert game is not None
    champion = find_champion()
    if game.resigned_by is None and not game.board.is_game_over(claim_draw=True) and game.board.turn != game.human_colour:
        move = choose_move(game.board, champion.weights)
        game.moves.append(game.board.san(move))
        game.board.push(move)


@app.get("/")
def index() -> FileResponse:
    return FileResponse(Path(__file__).parent / "static" / "index.html")


@app.get("/api/dashboard")
def dashboard() -> dict:
    champion = find_champion()
    return {"champion": champion.__dict__ | {"weights": list(champion.weights)},
            "leaders": generation_leaders(champion.run)}


@app.post("/api/new-game")
def new_game(request: NewGame) -> dict:
    global game
    game = Game(chess.Board(), chess.WHITE if request.human_colour == "white" else chess.BLACK, [])
    let_bot_play()
    return game_payload()


@app.post("/api/move")
def player_move(request: PlayerMove) -> dict:
    if game is None:
        raise HTTPException(409, "Start a new game first.")
    if game.resigned_by is not None:
        raise HTTPException(409, "This game has ended by resignation. Start a new game.")
    if game.board.turn != game.human_colour:
        raise HTTPException(409, "Wait for the bot to move.")
    try:
        move = chess.Move.from_uci(request.move)
    except ValueError as error:
        raise HTTPException(400, "Move must use UCI notation, such as e2e4.") from error
    if move not in game.board.legal_moves:
        raise HTTPException(400, "That is not a legal move in this position.")
    game.moves.append(game.board.san(move))
    game.board.push(move)
    return game_payload()


@app.post("/api/bot-move")
def bot_move() -> dict:
    """Make the bot move in a separate request so its turn is visible in the UI."""
    if game is None:
        raise HTTPException(409, "Start a new game first.")
    if game.resigned_by is not None:
        return game_payload()
    if game.board.is_game_over(claim_draw=True):
        return game_payload()
    if game.board.turn == game.human_colour:
        raise HTTPException(409, "It is your turn.")
    let_bot_play()
    return game_payload()


@app.post("/api/resign")
def resign() -> dict:
    if game is None:
        raise HTTPException(409, "Start a new game first.")
    if game.resigned_by is None and not game.board.is_game_over(claim_draw=True):
        game.resigned_by = game.human_colour
    return game_payload()


@app.post("/api/rematch")
def rematch() -> dict:
    """Restart with the same human colour as the just-finished game."""
    global game
    if game is None:
        raise HTTPException(409, "Start a new game first.")
    game = Game(chess.Board(), game.human_colour, [])
    let_bot_play()
    return game_payload()

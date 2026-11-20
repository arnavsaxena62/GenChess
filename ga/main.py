"""Genetic training loop for the chess evaluation weights.

This module deliberately owns only the Python GA orchestration. Game play is
delegated to the existing ``chessbots`` C++ extension.
"""

from __future__ import annotations

import argparse
import csv
import json
import random
import sys
from dataclasses import asdict, dataclass
from datetime import datetime
from pathlib import Path
from typing import Callable, Iterable, Sequence

try:
    import chessbots
except ImportError as error:
    chessbots = None
    CHESSBOTS_IMPORT_ERROR = error
else:
    CHESSBOTS_IMPORT_ERROR = None


WEIGHT_NAMES = ("material", "pawn_structure", "mobility", "king_safety", "controlled_squares")
DEFAULT_BASELINE = (1.0, 1.0, 1.0, 1.0, 1.0)


@dataclass
class Candidate:
    """One evaluation function and its rating within the current generation."""

    identifier: int
    weights: list[float]
    elo: float = 1200.0
    wins: int = 0
    losses: int = 0
    draws: int = 0


@dataclass
class MatchRecord:
    generation: int
    round_number: int
    white_id: int
    black_id: int
    result: str
    white_elo_before: float
    black_elo_before: float
    white_elo_after: float
    black_elo_after: float


MatchRunner = Callable[[list[list[float]], list[tuple[int, int]]], Iterable[tuple[int, int, int]]]


class GeneticTrainer:
    def __init__(self, population_size: int, rounds: int, survivors: int, mutation_rate: float,
                 mutation_scale: float, initial_elo: float, k_factor: float, weight_min: float,
                 weight_max: float, seed: int | None, match_runner: MatchRunner) -> None:
        if population_size < 2:
            raise ValueError("population size must be at least 2")
        if not 1 <= survivors <= population_size:
            raise ValueError("survivors must be between 1 and population size")
        if rounds < 1:
            raise ValueError("rounds must be at least 1")
        if not 0 <= mutation_rate <= 1:
            raise ValueError("mutation rate must be in [0, 1]")
        if weight_min > weight_max:
            raise ValueError("weight minimum cannot exceed maximum")
        self.population_size = population_size
        self.rounds = rounds
        self.survivors = survivors
        self.mutation_rate = mutation_rate
        self.mutation_scale = mutation_scale
        self.initial_elo = initial_elo
        self.k_factor = k_factor
        self.weight_min = weight_min
        self.weight_max = weight_max
        self.rng = random.Random(seed)
        self.match_runner = match_runner
        self._next_identifier = 0

    def make_initial_population(self) -> list[Candidate]:
        return [self._new_candidate(self._random_weights()) for _ in range(self.population_size)]

    def _new_candidate(self, weights: Sequence[float]) -> Candidate:
        candidate = Candidate(self._next_identifier, list(weights), self.initial_elo)
        self._next_identifier += 1
        return candidate

    def _random_weights(self) -> list[float]:
        return [self.rng.uniform(self.weight_min, self.weight_max) for _ in WEIGHT_NAMES]

    def pair_by_elo(self, population: Sequence[Candidate], generation: int,
                    round_number: int) -> list[tuple[int, int]]:
        """Pair adjacent Elo ratings, alternating White to limit colour bias.

        Ratings are captured at the beginning of a round. They are updated after
        every game, and therefore determine the following round's ordering.
        """
        ordered = sorted(range(len(population)), key=lambda i: (-population[i].elo, population[i].identifier))
        pairs: list[tuple[int, int]] = []
        for offset in range(0, len(ordered) - 1, 2):
            first, second = ordered[offset], ordered[offset + 1]
            if (generation + round_number + offset // 2) % 2:
                first, second = second, first
            pairs.append((first, second))
        return pairs

    def play_generation(self, population: list[Candidate], generation: int) -> list[MatchRecord]:
        for candidate in population:
            candidate.elo = self.initial_elo
            candidate.wins = candidate.losses = candidate.draws = 0
        records: list[MatchRecord] = []
        for round_number in range(1, self.rounds + 1):
            pairs = self.pair_by_elo(population, generation, round_number)
            results = list(self.match_runner([candidate.weights for candidate in population], pairs))
            if len(results) != len(pairs):
                raise RuntimeError(f"match runner returned {len(results)} results for {len(pairs)} requested matches")
            for white_index, black_index, outcome in results:
                if not (0 <= white_index < len(population) and 0 <= black_index < len(population)) or white_index == black_index:
                    raise RuntimeError("match runner returned invalid candidate indexes")
                records.append(self._apply_result(population[white_index], population[black_index], outcome,
                                                  generation, round_number))
        return records

    def _apply_result(self, white: Candidate, black: Candidate, outcome: int, generation: int,
                      round_number: int) -> MatchRecord:
        if outcome == 0:
            white_score, result = 1.0, "white_win"
            white.wins += 1
            black.losses += 1
        elif outcome == 1:
            white_score, result = 0.0, "black_win"
            white.losses += 1
            black.wins += 1
        elif outcome == 2:
            white_score, result = 0.5, "draw"
            white.draws += 1
            black.draws += 1
        else:
            raise RuntimeError(f"unknown match result {outcome!r}; expected 0, 1, or 2")
        white_before, black_before = white.elo, black.elo
        expected_white = 1.0 / (1.0 + 10 ** ((black_before - white_before) / 400.0))
        change = self.k_factor * (white_score - expected_white)
        white.elo += change
        black.elo -= change
        return MatchRecord(generation, round_number, white.identifier, black.identifier, result,
                           white_before, black_before, white.elo, black.elo)

    def ranked(self, population: Sequence[Candidate]) -> list[Candidate]:
        return sorted(population, key=lambda candidate: (-candidate.elo, candidate.identifier))

    def reproduce(self, population: Sequence[Candidate]) -> list[Candidate]:
        """Elitism plus crossover/mutation from the top Elo candidates."""
        parents = self.ranked(population)[:self.survivors]
        next_population = [self._new_candidate(parent.weights) for parent in parents]
        while len(next_population) < self.population_size:
            left, right = self.rng.sample(parents, k=2) if len(parents) > 1 else (parents[0], parents[0])
            child = [self.rng.choice((a, b)) for a, b in zip(left.weights, right.weights)]
            child = [self._mutate(weight) for weight in child]
            next_population.append(self._new_candidate(child))
        return next_population

    def _mutate(self, weight: float) -> float:
        if self.rng.random() < self.mutation_rate:
            weight += self.rng.gauss(0.0, self.mutation_scale)
        return min(self.weight_max, max(self.weight_min, weight))


def write_csv(path: Path, rows: Sequence[dict]) -> None:
    if rows:
        with path.open("w", newline="", encoding="utf-8") as file:
            writer = csv.DictWriter(file, fieldnames=rows[0].keys())
            writer.writeheader()
            writer.writerows(rows)


def compare_with_baseline(champion: Candidate, baseline: Sequence[float], games: int,
                          match_runner: MatchRunner) -> dict:
    """Play colour-balanced champion-vs-manual-baseline matches after training."""
    pairs = [(0, 1) if game % 2 == 0 else (1, 0) for game in range(games)]
    results = list(match_runner([champion.weights, list(baseline)], pairs))
    score = 0.0
    matches = []
    for game, (white, _black, outcome) in enumerate(results, start=1):
        champion_is_white = white == 0
        champion_score = ((1.0 if outcome == 0 else 0.0 if outcome == 1 else 0.5)
                          if champion_is_white else (1.0 if outcome == 1 else 0.0 if outcome == 0 else 0.5))
        score += champion_score
        matches.append({"game": game, "champion_colour": "white" if champion_is_white else "black",
                        "result_code": outcome, "champion_score": champion_score})
    return {"champion_id": champion.identifier, "champion_weights": champion.weights,
            "baseline_weights": list(baseline), "games": games, "champion_score": score,
            "baseline_score": games - score, "champion_score_percent": 100.0 * score / games,
            "matches": matches}


def parse_weights(value: str) -> list[float]:
    weights = [float(part.strip()) for part in value.split(",")]
    if len(weights) != len(WEIGHT_NAMES):
        raise argparse.ArgumentTypeError(f"baseline needs exactly {len(WEIGHT_NAMES)} comma-separated weights")
    return weights


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Evolve chess evaluation weights using Elo-seeded genetic training.")
    parser.add_argument("--generations", type=int, default=10)
    parser.add_argument("--population", type=int, default=30)
    parser.add_argument("--rounds", type=int, default=10, help="Elo-seeded match rounds per generation (default: 10)")
    parser.add_argument("--survivors", type=int, default=5, help="top Elo reproducers (default: 5)")
    parser.add_argument("--mutation-rate", type=float, default=0.20)
    parser.add_argument("--mutation-scale", type=float, default=0.15)
    parser.add_argument("--initial-elo", type=float, default=1200.0)
    parser.add_argument("--k-factor", type=float, default=32.0)
    parser.add_argument("--weight-min", type=float, default=0.0)
    parser.add_argument("--weight-max", type=float, default=2.0)
    parser.add_argument("--baseline", type=parse_weights, default=list(DEFAULT_BASELINE),
                        help="manual weights, e.g. 1,1,1,1,1")
    parser.add_argument("--baseline-games", type=int, default=10)
    parser.add_argument("--seed", type=int, default=None)
    parser.add_argument("--output-dir", type=Path, default=None)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if chessbots is None:
        print("Could not import chessbots. Build it first with: python compile-cpp/setup.py build_ext --inplace", file=sys.stderr)
        print(CHESSBOTS_IMPORT_ERROR, file=sys.stderr)
        return 2
    if args.generations < 1 or args.baseline_games < 1:
        print("--generations and --baseline-games must both be at least 1", file=sys.stderr)
        return 2
    trainer = GeneticTrainer(args.population, args.rounds, args.survivors, args.mutation_rate,
                             args.mutation_scale, args.initial_elo, args.k_factor, args.weight_min,
                             args.weight_max, args.seed, chessbots.batch_matches_defined)
    run_dir = args.output_dir or Path("runs") / datetime.now().strftime("%Y%m%d-%H%M%S")
    run_dir.mkdir(parents=True, exist_ok=False)
    population = trainer.make_initial_population()
    all_matches: list[dict] = []
    all_generations: list[dict] = []
    for generation in range(1, args.generations + 1):
        print(f"Generation {generation}/{args.generations}: {args.rounds} Elo-seeded rounds")
        all_matches.extend(asdict(record) for record in trainer.play_generation(population, generation))
        ranked = trainer.ranked(population)
        for rank, candidate in enumerate(ranked, start=1):
            all_generations.append({"generation": generation, "candidate_id": candidate.identifier,
                                    "rank": rank, "elo": candidate.elo, "wins": candidate.wins,
                                    "losses": candidate.losses, "draws": candidate.draws,
                                    "weights": json.dumps(candidate.weights)})
        champion = ranked[0]
        print(f"  best: id={champion.identifier}, Elo={champion.elo:.1f}, weights={champion.weights}")
        if generation < args.generations:
            population = trainer.reproduce(population)
    champion = trainer.ranked(population)[0]
    comparison = compare_with_baseline(champion, args.baseline, args.baseline_games, chessbots.batch_matches_defined)
    write_csv(run_dir / "matches.csv", all_matches)
    write_csv(run_dir / "generations.csv", all_generations)
    with (run_dir / "baseline_comparison.json").open("w", encoding="utf-8") as file:
        json.dump(comparison, file, indent=2)
    with (run_dir / "config.json").open("w", encoding="utf-8") as file:
        json.dump(vars(args) | {"output_dir": str(run_dir)}, file, indent=2, default=str)
    print(f"\nTraining complete. Champion id={champion.identifier}, Elo={champion.elo:.1f}")
    print(f"Baseline comparison: {comparison['champion_score']}/{comparison['games']} ({comparison['champion_score_percent']:.1f}%)")
    print(f"Logs written to {run_dir}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except KeyboardInterrupt:
        print("\nTraining cancelled by user.")
        raise SystemExit(130)

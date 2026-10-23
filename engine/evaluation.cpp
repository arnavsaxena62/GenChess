#include "evaluation.h"
#include "movegen.h"

static MoveGen movegen;

namespace {

// Tables built once at program start
struct EvalTables {
    u64 fileMask[8];
    u64 adjacentFiles[8];
    u64 passedMask[2][64]; // [color][square]: squares ahead on own + neighbouring files

    EvalTables() {
        for (int f = 0; f < 8; f++) {
            fileMask[f] = 0x0101010101010101ULL << f;
        }
        for (int f = 0; f < 8; f++) {
            u64 adj = 0;
            if (f > 0) adj |= fileMask[f - 1];
            if (f < 7) adj |= fileMask[f + 1];
            adjacentFiles[f] = adj;
        }
        for (int sq = 0; sq < 64; sq++) {
            int rank = sq / 8;
            int file = sq % 8;
            u64 files = fileMask[file] | adjacentFiles[file];

            u64 ahead = 0;
            for (int r = rank + 1; r < 8; r++)
                ahead |= 0xFFULL << (r * 8);
            passedMask[WHITE][sq] = files & ahead;

            u64 behind = 0;
            for (int r = rank - 1; r >= 0; r--)
                behind |= 0xFFULL << (r * 8);
            passedMask[BLACK][sq] = files & behind;
        }
    }
};

const EvalTables T;

inline int kingSquare(u64 king) {
    return king == 0 ? -1 : __builtin_ctzll(king);
}

double isolatedPawnScore(u64 pawns) {
    int isolated = 0;
    for (int file = 0; file < 8; file++) {
        u64 currentFile = pawns & T.fileMask[file];
        if (currentFile && !(pawns & T.adjacentFiles[file]))
            isolated += __builtin_popcountll(currentFile);
    }
    return -static_cast<double>(isolated);
}

double doubledPawnScore(u64 pawns) {
    int doubled = 0;
    for (int file = 0; file < 8; file++) {
        int n = __builtin_popcountll(pawns & T.fileMask[file]);
        if (n > 1)
            doubled += n - 1;
    }
    return -static_cast<double>(doubled);
}

double passedPawnScore(u64 friendlyPawns, u64 enemyPawns, Color color) {
    int passed = 0;
    while (friendlyPawns) {
        int sq = __builtin_ctzll(friendlyPawns);
        friendlyPawns &= friendlyPawns - 1;
        if (!(enemyPawns & T.passedMask[color][sq]))
            passed++;
    }
    return static_cast<double>(passed);
}

double pawnShieldScore(const Position& position, int kingSq, Color color) {
    if (kingSq == -1)
        return 0.0;

    int kingRank = kingSq / 8;
    int kingFile = kingSq % 8;
    int pawnRank = (color == WHITE) ? kingRank + 1 : kingRank - 1;

    if (pawnRank < 0 || pawnRank > 7)
        return 0.0;

    u64 friendlyPawns = (color == WHITE) ? position.whitePawn : position.blackPawn;
    double score = 0.0;

    for (int file = kingFile - 1; file <= kingFile + 1; file++) {
        if (file < 0 || file > 7)
            continue;
        if (friendlyPawns & (1ULL << (pawnRank * 8 + file)))
            score += 1.0;
    }
    return score;
}

} // namespace

double materialScore(const Position& position) {
    double white =
        __builtin_popcountll(position.whitePawn) * 1.0 +
        __builtin_popcountll(position.whiteKnight) * 3.0 +
        __builtin_popcountll(position.whiteBishop) * 3.0 +
        __builtin_popcountll(position.whiteRook) * 5.0 +
        __builtin_popcountll(position.whiteQueen) * 9.0;

    double black =
        __builtin_popcountll(position.blackPawn) * 1.0 +
        __builtin_popcountll(position.blackKnight) * 3.0 +
        __builtin_popcountll(position.blackBishop) * 3.0 +
        __builtin_popcountll(position.blackRook) * 5.0 +
        __builtin_popcountll(position.blackQueen) * 9.0;

    return white - black;
}

double pawnStructureScore(const Position& position) {
    double white = doubledPawnScore(position.whitePawn) +
                   isolatedPawnScore(position.whitePawn) +
                   passedPawnScore(position.whitePawn, position.blackPawn, WHITE);

    double black = doubledPawnScore(position.blackPawn) +
                   isolatedPawnScore(position.blackPawn) +
                   passedPawnScore(position.blackPawn, position.whitePawn, BLACK);

    return white - black;
}

// Approximate mobility: squares each side controls, minus squares holding its own pieces.
// No move generation, no vectors, no position copies.
double mobilityScore(Position& position, u64 whiteControlled, u64 blackControlled) {
    int whiteMoves = __builtin_popcountll(whiteControlled & ~position.white());
    int blackMoves = __builtin_popcountll(blackControlled & ~position.black());
    return static_cast<double>(whiteMoves - blackMoves);
}

double kingSafetyScore(Position& position, u64 whiteControlled, u64 blackControlled) {
    int whiteKingSq = kingSquare(position.whiteKing);
    int blackKingSq = kingSquare(position.blackKing);

    double whiteScore = 0.0;
    double blackScore = 0.0;

    if (whiteKingSq != -1) {
        whiteScore += pawnShieldScore(position, whiteKingSq, WHITE);
        // enemy-controlled squares around my king
        whiteScore -= __builtin_popcountll(movegen.KingZone(whiteKingSq) & blackControlled);
    }
    if (blackKingSq != -1) {
        blackScore += pawnShieldScore(position, blackKingSq, BLACK);
        blackScore -= __builtin_popcountll(movegen.KingZone(blackKingSq) & whiteControlled);
    }

    return whiteScore - blackScore;
}

double controlledSquaresScore(u64 whiteControlled, u64 blackControlled) {
    return static_cast<double>(__builtin_popcountll(whiteControlled) - __builtin_popcountll(blackControlled));
}

double evaluate(Position& position, const evaluationWeights& weights, Color side) {
    evalCount++;

    double score = 0.0;

    if (weights.material != 0.0)
        score += weights.material * materialScore(position);

    if (weights.pawnStructure != 0.0)
        score += weights.pawnStructure * pawnStructureScore(position);

    // The three terms below all need the controlled-square bitboards,
    // so compute them once, and only if at least one of them is used.
    if (weights.mobility != 0.0 || weights.kingSafety != 0.0 ||
        weights.controlledSquares != 0.0) {

        u64 whiteControlled = movegen.ComputeControlledSquares(position, WHITE);
        u64 blackControlled = movegen.ComputeControlledSquares(position, BLACK);

        if (weights.mobility != 0.0)
            score += weights.mobility * mobilityScore(position, whiteControlled, blackControlled);

        if (weights.kingSafety != 0.0)
            score += weights.kingSafety * kingSafetyScore(position, whiteControlled, blackControlled);

        if (weights.controlledSquares != 0.0)
            score += weights.controlledSquares * controlledSquaresScore(whiteControlled, blackControlled);
    }

    return (side == BLACK) ? -score : score;
}
# Genetic-algorithm training loop

This folder contains only the Python orchestration for the genetic-algorithm component. It does not change the chess engine or evaluation function; games are run through the existing `chessbots` C++ extension.

For every generation the trainer:

1. creates Elo-adjacent pairings for **10 rounds** (the default);
2. updates Elo immediately after every game, so the next round uses the latest ratings;
3. ranks the population by Elo;
4. retains the **top 5** candidates as the reproduction pool, then fills the next population with crossover and mutation; and
5. plays the best evolved weights against the configurable manual baseline with alternating colours.

An odd population leaves the lowest-rated candidate with a bye for that round. Elo starts fresh each generation, making rankings comparable within the generation.

## Run

From this `ga` folder, build the existing extension once (skip this if `chessbots*.pyd` already exists):

```powershell
python -m pip install -e .
python compile-cpp/setup.py build_ext --inplace
```

Then run training:

```powershell
python main.py --generations 10 --seed 42
```

The defaults are a population of 30, 10 Elo-seeded rounds per generation, and five reproducers. A full run is computationally expensive because each match invokes the C++ chess engine. Start with a small smoke test if needed:

```powershell
python main.py --generations 1 --population 6 --rounds 10 --seed 42
```

Useful options:

```powershell
python main.py --generations 20 --population 30 --rounds 10 --survivors 5 --mutation-rate 0.2 --mutation-scale 0.15 --baseline 1,1,1,1,1
```

`--baseline` supplies the manually chosen weights in engine order: material, pawn structure, mobility, king safety, controlled squares.

## Logs

Each run creates `runs/<timestamp>/` containing:

- `matches.csv` - every match, result, and Elo before/after;
- `generations.csv` - rankings, records, and weights for every generation;
- `baseline_comparison.json` - colour-balanced final comparison with the manual baseline; and
- `config.json` - exact run settings.

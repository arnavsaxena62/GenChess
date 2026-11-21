# GenChess UI

This isolated local web app reads `../ga/runs/*/generations.csv`, finds the
highest-Elo generated candidate, and lets a person play it. It does not modify
the GA trainer, C++ engine, evaluation code, or saved logs.

## Run

From the repository root:

```powershell
python -m pip install -r ui/requirements.txt
python -m uvicorn ui.app:app --reload
```

Open <http://127.0.0.1:8000>. The dashboard presents the champion's source,
weights, and a graph of generation leaders from the champion's saved run. Choose White or Black to
start. Click the source and destination squares for each move; promotion is to
a queen automatically.

The C++ extension currently exposes batch bot matches only, not an interactive
position/move API. The UI therefore keeps it untouched and uses a local
`python-chess` adapter for legal human moves and a small alpha-beta search whose
five evaluation weights are exactly those read from the winning GA record.

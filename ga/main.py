import chessbots
import random
import pprint


def main():
    arr = [[random.randint(0, 200)/100 for _ in range(5)] for _ in range(30)]
    matches = [(1, 2), (2, 2), (3, 2)]
    try:
        result = chessbots.batch_matches_defined(arr, matches)
        pprint.pprint(result)
    except KeyboardInterrupt:
        print("\nTournament cancelled by user.")


if __name__ == "__main__":
    main()

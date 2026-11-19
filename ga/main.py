import chessbots
import random
import pprint
def main():
    arr = [[random.randint(0, 200)/100 for _ in range(5)] for _ in range(10)]
    result = chessbots.batch_matches(arr)
    pprint.pprint(result)

if __name__ == "__main__":
    main()

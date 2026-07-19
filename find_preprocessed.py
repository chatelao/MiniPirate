import sys

def main():
    try:
        with open("preprocessed.txt", "r") as f:
            lines = f.readlines()
    except Exception as e:
        print("Error opening file:", e)
        return

    for idx, line in enumerate(lines):
        if "printProgramString" in line:
            start = max(0, idx - 10)
            end = min(len(lines), idx + 10)
            print(f"--- Line {idx+1} ---")
            for i in range(start, end):
                print(f"{i+1}: {lines[i]}", end="")

if __name__ == "__main__":
    main()

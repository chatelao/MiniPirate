import sys

def main():
    try:
        with open("preprocessed_baseIO.txt", "r") as f:
            lines = f.readlines()
    except Exception as e:
        print("Error opening file:", e)
        return

    for idx, line in enumerate(lines):
        # We want to see where Print is declared or defined
        # We can search for lines containing "class Print" or "struct Print" or similar
        # or occurrences in a declaration
        if "class Print" in line or "struct Print" in line or "typedef" in line and "Print" in line:
            start = max(0, idx - 5)
            end = min(len(lines), idx + 5)
            print(f"--- Line {idx+1} ---")
            for i in range(start, end):
                print(f"{i+1}: {lines[i]}", end="")

if __name__ == "__main__":
    main()

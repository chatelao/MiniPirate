import sys

def main():
    try:
        with open("preprocessed.txt", "r") as f:
            content = f.read()
        print("Size of preprocessed.txt:", len(content))
        if len(content) > 0:
            print("First 200 chars:", content[:200])
    except Exception as e:
        print("Error:", e)

if __name__ == "__main__":
    main()

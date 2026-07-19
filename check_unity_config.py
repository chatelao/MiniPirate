import os

def check_file(path):
    print("Checking path:", path)
    if os.path.exists(path):
        print("  Exists! Size:", os.path.getsize(path))
        try:
            with open(path, "r") as f:
                print("  Content:")
                print(f.read())
        except Exception as e:
            print("  Read error:", e)
    else:
        print("  Does NOT exist!")

def main():
    check_file(".pio/build/native/unity_config/unity_config.c")
    check_file(".pio/build/native/unity_config/unity_config.h")

if __name__ == "__main__":
    main()

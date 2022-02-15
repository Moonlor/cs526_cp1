import os

if __name__ == '__main__':
    file = open(os.environ['FILE_NAME']).read()
    passed = True
    for id, line in enumerate(file.split('\n')):
        if "alloca %struct." in line:
            print(f"transformation failed for {os.environ['FILE_NAME']}:{id+1}: {line}")
            passed = False

    if passed:
        print("passed")


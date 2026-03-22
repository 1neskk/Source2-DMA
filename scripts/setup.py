import os
import subprocess
import platform
import shutil

def run_command(command):
    process = subprocess.Popen(command, shell=True)
    process.wait()

def main():
    cwd = os.getcwd()
    platform_name = platform.system()

    if os.path.exists(cwd + "/../build"):
        shutil.rmtree(cwd + "/../build", ignore_errors=True)
        print("Removed existing build directory.")
        os.mkdir(cwd + "/../build")
    else:
        os.mkdir(cwd + "/../build")

    os.chdir(cwd + "/../build")

    if platform_name == "Windows":
        print("Running CMake for Windows...\n")
        run_command("cmake -G \"Visual Studio 17 2022\" -A x64 ..")
        print("\nOpen the project in Visual Studio and build it.")
    else:
        run_command("cmake ..")

    os.chdir(cwd)

if __name__ == "__main__":
    main()
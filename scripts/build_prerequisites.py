import os
import subprocess
import sys

def main():
    # Check if the expected number of command-line arguments is provided
    if len(sys.argv) != 3:
        print("Usage: python build_prerequisites.py <root_build_dir>")
        sys.exit(1)

    # Extract the root_build_dir from the command-line arguments
    root_build_dir = sys.argv[1]

    # Extract the architecture from command-line arguments
    arch = sys.argv[2]

    if arch == "x64":
        arch = "amd64"

    # Get the file directory
    file_directory = os.path.dirname(os.path.abspath(__file__))

    # Specify the name of your shell script
    script_name = 'build_prerequisites.sh'

    # Construct the full path to the shell script
    script_path = os.path.join(file_directory, script_name)

    # Run the shell script with the provided root_build_dir argument
    subprocess.run(['bash', script_path, root_build_dir, arch])

if __name__ == "__main__":
    main()
import gzip
import os
from SCons.Script import DefaultEnvironment

Import("env")

# Input and output directories
input_path = "./data/easyWifi"  # Folder where the input files are located
output_path = "./src/"          # Folder where the .h file will be saved
output_header = os.path.join(output_path, "frontend.h")  # Name of the final .h file

# Function to compress a file
def compress_file(file_path):
    compressed_path = file_path + ".gz"
    with open(file_path, "rb") as f_in:
        with gzip.open(compressed_path, "wb") as f_out:
            f_out.writelines(f_in)
    return compressed_path

# Function to convert a .gz file to a C array
def file_to_c_array(file_path):
    array_name = os.path.basename(file_path).replace(".", "_").replace("-", "_")
    with open(file_path, "rb") as f:
        data = f.read()
    byte_array = ", ".join(f"0x{byte:02x}" for byte in data)
    length = len(data)
    c_array = f"const uint8_t {array_name}[] PROGMEM= {{ {byte_array} }};\n"
    c_array += f"unsigned int {array_name}_len = {length};\n\n"
    return c_array

# Main process
def main():
    # Ensure the output directory exists
    os.makedirs(output_path, exist_ok=True)

    # Get files from the input directory
    files = [os.path.join(input_path, f) for f in os.listdir(input_path) if os.path.isfile(os.path.join(input_path, f))]
    if not files:
        print("No files found in the input directory.")
        return

    # Generate the .h file
    with open(output_header, "w") as header_file:
        header_file.write("#include <pgmspace.h> //for PROGMEM\n\n")
        for file in files:
            print(f"Processing: {file}")
            compressed_file = compress_file(file)
            c_array = file_to_c_array(compressed_file)
            header_file.write(c_array)
            os.remove(compressed_file)  # Remove the temporary .gz file
        print(f"Header file generated: {output_header}")

main()
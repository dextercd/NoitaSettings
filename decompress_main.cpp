#include <iostream>
#include <string>

#include <fastlz.h>

#include "binops.hpp"
#include "utils.hpp"

int main(int argc, char** argv)
{
    if (argc != 2) {
        const char* program_path = argv[0];
        if (program_path[0] == '\0')
            program_path = "noita-decompress";

        std::cerr << "Bad usage!\n";
        std::cerr << "Example:\n  ";
        std::cerr << program_path << " <path to compressed_file>" << '\n';
        return 1;
    }

    auto decompressed_data = read_compressed_file(argv[1]);
    std::cout << decompressed_data;
}

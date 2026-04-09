#include "nnue_file.h"
#include <fstream>
#include <iostream>

namespace NNUE {

std::string load_nnue_file(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) return "Cannot open file: " + path;

    NNUEHeader header{};
    f.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (f.gcount() < (std::streamsize)sizeof(header))
        return "File too small (< 16 bytes)";

    f.seekg(0, std::ios::end);
    std::streamsize file_size = f.tellg();
    if (file_size < 16) return "File too small";

    std::cout << "info string NNUE header read successfully, format version "
              << header.version << std::endl;
    return "";
}

} // namespace NNUE

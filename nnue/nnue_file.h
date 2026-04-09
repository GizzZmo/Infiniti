#pragma once
#include <string>
#include <cstdint>

namespace NNUE {
constexpr uint32_t NNUE_MAGIC = 0x9D7B5A12;
constexpr uint32_t HALFKP_HEADER = 0x5D69D7B8;

struct NNUEHeader {
    uint32_t magic;
    uint32_t version;
    uint32_t hash;
    uint32_t desc_len;
};

// Returns empty string on success, error message on failure
std::string load_nnue_file(const std::string& path);
}

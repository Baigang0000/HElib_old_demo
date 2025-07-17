#pragma once
#include <vector>
#include <cstdint>

// Simple BCH(127,106,3) encoder/decoder for binary messages
class BCH {
public:
    static constexpr int n = 127;
    static constexpr int k = 106;
    static constexpr int t = 3;
    // Encode a 106-bit message to a 127-bit codeword
    std::vector<uint8_t> encode(const std::vector<uint8_t>& msg) const;
    // Decode a 127-bit codeword, correct up to 3 errors, return decoded 106-bit message
    // Returns true if decoding succeeded, false if too many errors
    bool decode(const std::vector<uint8_t>& codeword, std::vector<uint8_t>& decoded) const;
};
#include "bch.h"
#include <vector>
#include <cstdint>
#include <cstring>
#include <algorithm>

// This is a minimal, self-contained BCH(127,106,3) implementation for demonstration.
// For production, use a well-tested library. This code is for experiment only.

namespace {
// Generator polynomial for BCH(127,106,3): x^21 + ...
// Precomputed for (n=127, t=3)
const uint32_t generator[22] = {
    1,0,1,1,1,0,0,0,1,1,0,1,1,1,1,0,0,1,1,0,1,1 // degree 21
};

// Helper: polynomial division mod 2
void poly_div(const std::vector<uint8_t>& dividend, const uint32_t* divisor, int deg, std::vector<uint8_t>& remainder) {
    remainder = dividend;
    for (int i = 0; i <= BCH::n - deg; ++i) {
        if (remainder[i]) {
            for (int j = 0; j <= deg; ++j)
                remainder[i + j] ^= divisor[j];
        }
    }
}
}

std::vector<uint8_t> BCH::encode(const std::vector<uint8_t>& msg) const {
    // Systematic encoding: c(x) = m(x)x^{n-k} + parity
    std::vector<uint8_t> codeword(n, 0);
    // Copy message to high bits
    for (int i = 0; i < k; ++i)
        codeword[i] = msg[i];
    // Shift message left by n-k
    std::vector<uint8_t> shifted(n, 0);
    for (int i = 0; i < k; ++i)
        shifted[i] = msg[i];
    std::rotate(shifted.begin(), shifted.begin() + (n - k), shifted.end());
    // Compute remainder
    std::vector<uint8_t> remainder;
    poly_div(shifted, generator, 21, remainder);
    // Parity bits are the last n-k bits of remainder
    for (int i = 0; i < n - k; ++i)
        codeword[k + i] = remainder[k + i];
    return codeword;
}

bool BCH::decode(const std::vector<uint8_t>& codeword, std::vector<uint8_t>& decoded) const {
    // For demonstration: just return the first k bits (no error correction)
    // Replace with Berlekamp-Massey + Chien search for real decoding
    decoded.assign(codeword.begin(), codeword.begin() + k);
    // TODO: implement error correction
    // For now, always return true (pretend decode always succeeds)
    return true;
}
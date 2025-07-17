#include "bch.h"
#include <vector>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <iostream>

// This is a minimal, self-contained BCH(127,106,3) implementation for demonstration.
// For production, use a well-tested library. This code is for experiment only.

namespace {
// Generator polynomial for BCH(127,106,3): x^21 + ...
// Precomputed for (n=127, t=3)
const uint32_t generator[22] = {
    1,0,1,1,1,0,0,0,1,1,0,1,1,1,1,0,0,1,1,0,1,1 // degree 21
};

// GF(2^7) primitive element alpha and its powers
const uint8_t alpha_powers[127] = {
    1, 2, 4, 8, 16, 32, 64, 65, 3, 6, 12, 24, 48, 96, 66, 5, 10, 20, 40, 80, 34, 68,
    9, 18, 36, 72, 17, 34, 68, 9, 18, 36, 72, 17, 34, 68, 9, 18, 36, 72, 17, 34, 68,
    9, 18, 36, 72, 17, 34, 68, 9, 18, 36, 72, 17, 34, 68, 9, 18, 36, 72, 17, 34, 68,
    9, 18, 36, 72, 17, 34, 68, 9, 18, 36, 72, 17, 34, 68, 9, 18, 36, 72, 17, 34, 68,
    9, 18, 36, 72, 17, 34, 68, 9, 18, 36, 72, 17, 34, 68, 9, 18, 36, 72, 17, 34, 68,
    9, 18, 36, 72, 17, 34, 68, 9, 18, 36, 72, 17, 34, 68, 9, 18, 36, 72, 17, 34, 68
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

// GF(2^7) multiplication
uint8_t gf_mult(uint8_t a, uint8_t b) {
    uint8_t result = 0;
    for (int i = 0; i < 7; ++i) {
        if (b & 1) result ^= a;
        a <<= 1;
        if (a & 0x80) a ^= 0x8D; // primitive polynomial x^7 + x^3 + x^2 + x + 1
        b >>= 1;
    }
    return result;
}

// Compute syndrome S_i = r(alpha^i)
uint8_t compute_syndrome(const std::vector<uint8_t>& received, int i) {
    uint8_t syndrome = 0;
    for (int j = 0; j < BCH::n; ++j) {
        if (received[j]) {
            uint8_t alpha_pow = 1;
            for (int k = 0; k < (i * j) % 127; ++k) {
                alpha_pow = gf_mult(alpha_pow, 2);
            }
            syndrome ^= alpha_pow;
        }
    }
    return syndrome;
}

// Berlekamp-Massey algorithm for finding error locator polynomial
bool berlekamp_massey(const std::vector<uint8_t>& syndromes, std::vector<uint8_t>& error_locator) {
    std::vector<uint8_t> C(BCH::t + 1, 0); // error locator polynomial
    std::vector<uint8_t> B(BCH::t + 1, 0); // previous error locator
    C[0] = 1;
    B[0] = 1;
    
    int L = 0; // degree of error locator
    uint8_t b = 1; // previous discrepancy
    
    for (int n = 0; n < 2 * BCH::t; ++n) {
        uint8_t d = syndromes[n]; // discrepancy
        for (int i = 1; i <= L; ++i) {
            d ^= gf_mult(C[i], syndromes[n - i]);
        }
        
        if (d != 0) {
            std::vector<uint8_t> T = C;
            for (int i = 0; i <= BCH::t - n + L; ++i) {
                C[n - L + i] ^= gf_mult(d, B[i]);
            }
            if (2 * L <= n) {
                L = n + 1 - L;
                B = T;
                b = d;
            }
        }
    }
    
    error_locator = C;
    return L <= BCH::t;
}

// Chien search to find error locations
std::vector<int> chien_search(const std::vector<uint8_t>& error_locator) {
    std::vector<int> error_positions;
    for (int i = 0; i < BCH::n; ++i) {
        uint8_t sum = 0;
        for (int j = 0; j < error_locator.size(); ++j) {
            uint8_t alpha_pow = 1;
            for (int k = 0; k < (j * i) % 127; ++k) {
                alpha_pow = gf_mult(alpha_pow, 2);
            }
            sum ^= gf_mult(error_locator[j], alpha_pow);
        }
        if (sum == 0) {
            error_positions.push_back(i);
        }
    }
    return error_positions;
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
    if (codeword.size() < n) {
        return false; // Invalid codeword length
    }
    
    // Compute syndromes S_1, S_2, ..., S_{2t}
    std::vector<uint8_t> syndromes(2 * t);
    for (int i = 1; i <= 2 * t; ++i) {
        syndromes[i-1] = compute_syndrome(codeword, i);
    }
    
    // Check if all syndromes are zero (no errors)
    bool no_errors = true;
    for (uint8_t s : syndromes) {
        if (s != 0) {
            no_errors = false;
            break;
        }
    }
    
    if (no_errors) {
        // No errors, just extract the message
        decoded.assign(codeword.begin(), codeword.begin() + k);
        return true;
    }
    
    // Find error locator polynomial using Berlekamp-Massey
    std::vector<uint8_t> error_locator;
    if (!berlekamp_massey(syndromes, error_locator)) {
        return false; // Too many errors
    }
    
    // Find error positions using Chien search
    std::vector<int> error_positions = chien_search(error_locator);
    
    if (error_positions.size() > t) {
        return false; // Too many errors
    }
    
    // Correct errors by flipping bits at error positions
    std::vector<uint8_t> corrected = codeword;
    for (int pos : error_positions) {
        if (pos < n) {
            corrected[pos] ^= 1;
        }
    }
    
    // Extract the message (first k bits)
    decoded.assign(corrected.begin(), corrected.begin() + k);
    return true;
}
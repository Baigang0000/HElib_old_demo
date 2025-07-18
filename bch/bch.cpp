#include "bch.h"
#include <vector>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <iostream>

// This is a minimal, self-contained BCH implementation for demonstration.
// For production, use a well-tested library. This code is for experiment only.

namespace {
// Generator polynomial for BCH(255,223,4): x^32 + ...
// Precomputed for (n=255, t=4)
const uint32_t generator_255[33] = {
    1,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,0,1 // degree 32
};

// Generator polynomial for BCH(127,106,3): x^21 + ...
// Precomputed for (n=127, t=3)
const uint32_t generator_127[22] = {
    1,0,1,1,1,0,0,0,1,1,0,1,1,1,1,0,0,1,1,0,1,1 // degree 21
};

// Helper: polynomial division mod 2
void poly_div(const std::vector<uint8_t>& dividend, const uint32_t* divisor, int deg, std::vector<uint8_t>& remainder) {
    remainder = dividend;
    for (int i = 0; i <= (int)dividend.size() - deg; ++i) {
        if (remainder[i]) {
            for (int j = 0; j <= deg; ++j)
                if (i + j < (int)remainder.size())
                    remainder[i + j] ^= divisor[j];
        }
    }
}

// GF(2^8) multiplication for BCH(255,223,4)
uint8_t gf_mult_8(uint8_t a, uint8_t b) {
    uint8_t result = 0;
    for (int i = 0; i < 8; ++i) {
        if (b & 1) result ^= a;
        a <<= 1;
        if (a & 0x100) a ^= 0x11D; // primitive polynomial x^8 + x^4 + x^3 + x^2 + 1
        b >>= 1;
    }
    return result;
}

// GF(2^7) multiplication for BCH(127,106,3)
uint8_t gf_mult_7(uint8_t a, uint8_t b) {
    uint8_t result = 0;
    for (int i = 0; i < 7; ++i) {
        if (b & 1) result ^= a;
        a <<= 1;
        if (a & 0x80) a ^= 0x8D; // primitive polynomial x^7 + x^3 + x^2 + x + 1
        b >>= 1;
    }
    return result;
}

// Compute syndrome S_i = r(alpha^i) for BCH(255,223,4)
uint8_t compute_syndrome_255(const std::vector<uint8_t>& received, int i) {
    uint8_t syndrome = 0;
    for (int j = 0; j < 255; ++j) {
        if (received[j]) {
            uint8_t alpha_pow = 1;
            for (int k = 0; k < (i * j) % 255; ++k) {
                alpha_pow = gf_mult_8(alpha_pow, 2);
            }
            syndrome ^= alpha_pow;
        }
    }
    return syndrome;
}

// Compute syndrome S_i = r(alpha^i) for BCH(127,106,3)
uint8_t compute_syndrome_127(const std::vector<uint8_t>& received, int i) {
    uint8_t syndrome = 0;
    for (int j = 0; j < 127; ++j) {
        if (received[j]) {
            uint8_t alpha_pow = 1;
            for (int k = 0; k < (i * j) % 127; ++k) {
                alpha_pow = gf_mult_7(alpha_pow, 2);
            }
            syndrome ^= alpha_pow;
        }
    }
    return syndrome;
}

// Berlekamp-Massey algorithm for finding error locator polynomial
bool berlekamp_massey(const std::vector<uint8_t>& syndromes, int t, std::vector<uint8_t>& error_locator) {
    std::vector<uint8_t> C(t + 1, 0); // error locator polynomial
    std::vector<uint8_t> B(t + 1, 0); // previous error locator
    C[0] = 1;
    B[0] = 1;
    
    int L = 0; // degree of error locator
    uint8_t b = 1; // previous discrepancy
    
    for (int n = 0; n < 2 * t; ++n) {
        uint8_t d = syndromes[n]; // discrepancy
        for (int i = 1; i <= L; ++i) {
            d ^= gf_mult_8(C[i], syndromes[n - i]);
        }
        
        if (d != 0) {
            std::vector<uint8_t> T = C;
            for (int i = 0; i <= t - n + L; ++i) {
                C[n - L + i] ^= gf_mult_8(d, B[i]);
            }
            if (2 * L <= n) {
                L = n + 1 - L;
                B = T;
                b = d;
            }
        }
    }
    
    error_locator = C;
    return L <= t;
}

// Chien search to find error locations for BCH(255,223,4)
std::vector<int> chien_search_255(const std::vector<uint8_t>& error_locator) {
    std::vector<int> error_positions;
    for (int i = 0; i < 255; ++i) {
        uint8_t sum = 0;
        for (int j = 0; j < error_locator.size(); ++j) {
            uint8_t alpha_pow = 1;
            for (int k = 0; k < (j * i) % 255; ++k) {
                alpha_pow = gf_mult_8(alpha_pow, 2);
            }
            sum ^= gf_mult_8(error_locator[j], alpha_pow);
        }
        if (sum == 0) {
            error_positions.push_back(i);
        }
    }
    return error_positions;
}

// Chien search to find error locations for BCH(127,106,3)
std::vector<int> chien_search_127(const std::vector<uint8_t>& error_locator) {
    std::vector<int> error_positions;
    for (int i = 0; i < 127; ++i) {
        uint8_t sum = 0;
        for (int j = 0; j < error_locator.size(); ++j) {
            uint8_t alpha_pow = 1;
            for (int k = 0; k < (j * i) % 127; ++k) {
                alpha_pow = gf_mult_7(alpha_pow, 2);
            }
            sum ^= gf_mult_7(error_locator[j], alpha_pow);
        }
        if (sum == 0) {
            error_positions.push_back(i);
        }
    }
    return error_positions;
}
}

BCH::BCH(int n_param, int k_param, int t_param) 
    : n_val(n_param), k_val(k_param), t_val(t_param) {
}

std::vector<uint8_t> BCH::encode(const std::vector<uint8_t>& msg) const {
    if (msg.size() != k_val) {
        std::cerr << "Message size " << msg.size() << " != k " << k_val << std::endl;
        return std::vector<uint8_t>();
    }
    
    // Systematic encoding: c(x) = m(x)x^{n-k} + parity
    std::vector<uint8_t> codeword(n_val, 0);
    
    // Copy message to high bits
    for (int i = 0; i < k_val; ++i)
        codeword[i] = msg[i];
    
    // Shift message left by n-k
    std::vector<uint8_t> shifted(n_val, 0);
    for (int i = 0; i < k_val; ++i)
        shifted[i] = msg[i];
    std::rotate(shifted.begin(), shifted.begin() + (n_val - k_val), shifted.end());
    
    // Compute remainder
    std::vector<uint8_t> remainder;
    if (n_val == 255) {
        poly_div(shifted, generator_255, 32, remainder);
    } else if (n_val == 127) {
        poly_div(shifted, generator_127, 21, remainder);
    } else {
        std::cerr << "Unsupported BCH code length: " << n_val << std::endl;
        return std::vector<uint8_t>();
    }
    
    // Parity bits are the last n-k bits of remainder
    for (int i = 0; i < n_val - k_val; ++i)
        codeword[k_val + i] = remainder[k_val + i];
    
    return codeword;
}

bool BCH::decode(const std::vector<uint8_t>& codeword, std::vector<uint8_t>& decoded) const {
    if (codeword.size() < n_val) {
        return false; // Invalid codeword length
    }
    
    // Compute syndromes S_1, S_2, ..., S_{2t}
    std::vector<uint8_t> syndromes(2 * t_val);
    for (int i = 1; i <= 2 * t_val; ++i) {
        if (n_val == 255) {
            syndromes[i-1] = compute_syndrome_255(codeword, i);
        } else if (n_val == 127) {
            syndromes[i-1] = compute_syndrome_127(codeword, i);
        } else {
            std::cerr << "Unsupported BCH code length: " << n_val << std::endl;
            return false;
        }
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
        decoded.assign(codeword.begin(), codeword.begin() + k_val);
        return true;
    }
    
    // Find error locator polynomial using Berlekamp-Massey
    std::vector<uint8_t> error_locator;
    if (!berlekamp_massey(syndromes, t_val, error_locator)) {
        return false; // Too many errors
    }
    
    // Find error positions using Chien search
    std::vector<int> error_positions;
    if (n_val == 255) {
        error_positions = chien_search_255(error_locator);
    } else if (n_val == 127) {
        error_positions = chien_search_127(error_locator);
    } else {
        return false;
    }
    
    if (error_positions.size() > t_val) {
        return false; // Too many errors
    }
    
    // Correct errors by flipping bits at error positions
    std::vector<uint8_t> corrected = codeword;
    for (int pos : error_positions) {
        if (pos < n_val) {
            corrected[pos] ^= 1;
        }
    }
    
    // Extract the message (first k bits)
    decoded.assign(corrected.begin(), corrected.begin() + k_val);
    return true;
}
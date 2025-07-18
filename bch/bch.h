#pragma once
#include <vector>
#include <cstdint>

// Configurable BCH encoder/decoder
class BCH {
public:
    // Default parameters for BCH(255,223,4)
    static constexpr int n = 255;
    static constexpr int k = 223;
    static constexpr int t = 4;
    
    // Constructor with custom parameters
    BCH(int n_param = n, int k_param = k, int t_param = t);
    
    // Encode a k-bit message to an n-bit codeword
    std::vector<uint8_t> encode(const std::vector<uint8_t>& msg) const;
    
    // Decode an n-bit codeword, correct up to t errors, return decoded k-bit message
    // Returns true if decoding succeeded, false if too many errors
    bool decode(const std::vector<uint8_t>& codeword, std::vector<uint8_t>& decoded) const;
    
    // Get current parameters
    int get_n() const { return n_val; }
    int get_k() const { return k_val; }
    int get_t() const { return t_val; }
    
private:
    int n_val, k_val, t_val;
};
#include <vector>
#include <random>
#include <iostream>
#include <algorithm>
#include "bch/bch.h"

int main() {
    BCH bch;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution d(0.5);
    
    std::cout << "Testing BCH(" << BCH::n << "," << BCH::k << "," << BCH::t << ") error correction\n";
    std::cout << "==========================================\n\n";
    
    // Test 1: No errors
    std::cout << "Test 1: No errors\n";
    std::vector<uint8_t> msg(BCH::k);
    for (int i = 0; i < BCH::k; ++i) msg[i] = d(gen);
    
    auto codeword = bch.encode(msg);
    std::vector<uint8_t> decoded;
    bool ok = bch.decode(codeword, decoded);
    std::cout << "Original message: ";
    for (int i = 0; i < std::min(10, BCH::k); ++i) std::cout << (int)msg[i];
    std::cout << "...\n";
    std::cout << "Decode success: " << ok << ", Message correct: " << (decoded == msg) << "\n\n";
    
    // Test 2: 1 bit error
    std::cout << "Test 2: 1 bit error\n";
    auto codeword2 = codeword;
    codeword2[0] ^= 1; // Flip first bit
    bool ok2 = bch.decode(codeword2, decoded);
    std::cout << "Decode success: " << ok2 << ", Message correct: " << (decoded == msg) << "\n\n";
    
    // Test 3: 2 bit errors
    std::cout << "Test 3: 2 bit errors\n";
    auto codeword3 = codeword;
    codeword3[0] ^= 1; // Flip first bit
    codeword3[10] ^= 1; // Flip 11th bit
    bool ok3 = bch.decode(codeword3, decoded);
    std::cout << "Decode success: " << ok3 << ", Message correct: " << (decoded == msg) << "\n\n";
    
    // Test 4: 3 bit errors
    std::cout << "Test 4: 3 bit errors\n";
    auto codeword4 = codeword;
    codeword4[0] ^= 1; // Flip first bit
    codeword4[10] ^= 1; // Flip 11th bit
    codeword4[20] ^= 1; // Flip 21st bit
    bool ok4 = bch.decode(codeword4, decoded);
    std::cout << "Decode success: " << ok4 << ", Message correct: " << (decoded == msg) << "\n\n";
    
    // Test 5: 4 bit errors (should fail)
    std::cout << "Test 5: 4 bit errors (should fail)\n";
    auto codeword5 = codeword;
    codeword5[0] ^= 1; // Flip first bit
    codeword5[10] ^= 1; // Flip 11th bit
    codeword5[20] ^= 1; // Flip 21st bit
    codeword5[30] ^= 1; // Flip 31st bit
    bool ok5 = bch.decode(codeword5, decoded);
    std::cout << "Decode success: " << ok5 << ", Message correct: " << (decoded == msg) << "\n\n";
    
    // Test 6: Random errors
    std::cout << "Test 6: Random error statistics\n";
    int total_trials = 1000;
    int success_0 = 0, success_1 = 0, success_2 = 0, success_3 = 0, success_4 = 0;
    
    for (int trial = 0; trial < total_trials; ++trial) {
        // Generate random message
        std::vector<uint8_t> test_msg(BCH::k);
        for (int i = 0; i < BCH::k; ++i) test_msg[i] = d(gen);
        
        auto test_codeword = bch.encode(test_msg);
        
        // Test with 0, 1, 2, 3, 4 errors
        for (int errors = 0; errors <= 4; ++errors) {
            auto noisy_codeword = test_codeword;
            std::vector<int> positions(BCH::n);
            for (int i = 0; i < BCH::n; ++i) positions[i] = i;
            std::shuffle(positions.begin(), positions.end(), gen);
            
            for (int i = 0; i < errors; ++i) {
                noisy_codeword[positions[i]] ^= 1;
            }
            
            std::vector<uint8_t> test_decoded;
            bool test_ok = bch.decode(noisy_codeword, test_decoded);
            
            if (test_ok && test_decoded == test_msg) {
                switch (errors) {
                    case 0: success_0++; break;
                    case 1: success_1++; break;
                    case 2: success_2++; break;
                    case 3: success_3++; break;
                    case 4: success_4++; break;
                }
            }
        }
    }
    
    std::cout << "Success rates:\n";
    std::cout << "0 errors: " << (double)success_0 / total_trials * 100 << "%\n";
    std::cout << "1 error:  " << (double)success_1 / total_trials * 100 << "%\n";
    std::cout << "2 errors: " << (double)success_2 / total_trials * 100 << "%\n";
    std::cout << "3 errors: " << (double)success_3 / total_trials * 100 << "%\n";
    std::cout << "4 errors: " << (double)success_4 / total_trials * 100 << "%\n";
    
    return 0;
}
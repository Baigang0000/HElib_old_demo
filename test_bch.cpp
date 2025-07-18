#include <vector>
#include <random>
#include <iostream>
#include <algorithm>
#include "bch/bch.h"

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution d(0.5);
    
    std::cout << "Testing BCH Error Correction\n";
    std::cout << "============================\n\n";
    
    // Test BCH(255,223,4)
    std::cout << "=== BCH(255,223,4) Tests ===\n";
    BCH bch_255(255, 223, 4);
    
    // Test 1: No errors
    std::cout << "Test 1: No errors\n";
    std::vector<uint8_t> msg_255(223);
    for (int i = 0; i < 223; ++i) msg_255[i] = d(gen);
    
    auto codeword_255 = bch_255.encode(msg_255);
    std::vector<uint8_t> decoded_255;
    bool ok_255 = bch_255.decode(codeword_255, decoded_255);
    std::cout << "Original message: ";
    for (int i = 0; i < std::min(10, 223); ++i) std::cout << (int)msg_255[i];
    std::cout << "...\n";
    std::cout << "Decode success: " << ok_255 << ", Message correct: " << (decoded_255 == msg_255) << "\n\n";
    
    // Test 2: 1 bit error
    std::cout << "Test 2: 1 bit error\n";
    auto codeword_255_1 = codeword_255;
    codeword_255_1[0] ^= 1; // Flip first bit
    bool ok_255_1 = bch_255.decode(codeword_255_1, decoded_255);
    std::cout << "Decode success: " << ok_255_1 << ", Message correct: " << (decoded_255 == msg_255) << "\n\n";
    
    // Test 3: 2 bit errors
    std::cout << "Test 3: 2 bit errors\n";
    auto codeword_255_2 = codeword_255;
    codeword_255_2[0] ^= 1; // Flip first bit
    codeword_255_2[10] ^= 1; // Flip 11th bit
    bool ok_255_2 = bch_255.decode(codeword_255_2, decoded_255);
    std::cout << "Decode success: " << ok_255_2 << ", Message correct: " << (decoded_255 == msg_255) << "\n\n";
    
    // Test 4: 4 bit errors
    std::cout << "Test 4: 4 bit errors\n";
    auto codeword_255_4 = codeword_255;
    codeword_255_4[0] ^= 1; // Flip first bit
    codeword_255_4[10] ^= 1; // Flip 11th bit
    codeword_255_4[20] ^= 1; // Flip 21st bit
    codeword_255_4[30] ^= 1; // Flip 31st bit
    bool ok_255_4 = bch_255.decode(codeword_255_4, decoded_255);
    std::cout << "Decode success: " << ok_255_4 << ", Message correct: " << (decoded_255 == msg_255) << "\n\n";
    
    // Test 5: 5 bit errors (should fail)
    std::cout << "Test 5: 5 bit errors (should fail)\n";
    auto codeword_255_5 = codeword_255;
    codeword_255_5[0] ^= 1; // Flip first bit
    codeword_255_5[10] ^= 1; // Flip 11th bit
    codeword_255_5[20] ^= 1; // Flip 21st bit
    codeword_255_5[30] ^= 1; // Flip 31st bit
    codeword_255_5[40] ^= 1; // Flip 41st bit
    bool ok_255_5 = bch_255.decode(codeword_255_5, decoded_255);
    std::cout << "Decode success: " << ok_255_5 << ", Message correct: " << (decoded_255 == msg_255) << "\n\n";
    
    // Test BCH(127,106,3)
    std::cout << "=== BCH(127,106,3) Tests ===\n";
    BCH bch_127(127, 106, 3);
    
    // Test 1: No errors
    std::cout << "Test 1: No errors\n";
    std::vector<uint8_t> msg_127(106);
    for (int i = 0; i < 106; ++i) msg_127[i] = d(gen);
    
    auto codeword_127 = bch_127.encode(msg_127);
    std::vector<uint8_t> decoded_127;
    bool ok_127 = bch_127.decode(codeword_127, decoded_127);
    std::cout << "Original message: ";
    for (int i = 0; i < std::min(10, 106); ++i) std::cout << (int)msg_127[i];
    std::cout << "...\n";
    std::cout << "Decode success: " << ok_127 << ", Message correct: " << (decoded_127 == msg_127) << "\n\n";
    
    // Test 2: 1 bit error
    std::cout << "Test 2: 1 bit error\n";
    auto codeword_127_1 = codeword_127;
    codeword_127_1[0] ^= 1; // Flip first bit
    bool ok_127_1 = bch_127.decode(codeword_127_1, decoded_127);
    std::cout << "Decode success: " << ok_127_1 << ", Message correct: " << (decoded_127 == msg_127) << "\n\n";
    
    // Test 3: 3 bit errors
    std::cout << "Test 3: 3 bit errors\n";
    auto codeword_127_3 = codeword_127;
    codeword_127_3[0] ^= 1; // Flip first bit
    codeword_127_3[10] ^= 1; // Flip 11th bit
    codeword_127_3[20] ^= 1; // Flip 21st bit
    bool ok_127_3 = bch_127.decode(codeword_127_3, decoded_127);
    std::cout << "Decode success: " << ok_127_3 << ", Message correct: " << (decoded_127 == msg_127) << "\n\n";
    
    // Test 4: 4 bit errors (should fail)
    std::cout << "Test 4: 4 bit errors (should fail)\n";
    auto codeword_127_4 = codeword_127;
    codeword_127_4[0] ^= 1; // Flip first bit
    codeword_127_4[10] ^= 1; // Flip 11th bit
    codeword_127_4[20] ^= 1; // Flip 21st bit
    codeword_127_4[30] ^= 1; // Flip 31st bit
    bool ok_127_4 = bch_127.decode(codeword_127_4, decoded_127);
    std::cout << "Decode success: " << ok_127_4 << ", Message correct: " << (decoded_127 == msg_127) << "\n\n";
    
    // Random error statistics for BCH(255,223,4)
    std::cout << "=== Random Error Statistics for BCH(255,223,4) ===\n";
    int total_trials = 100;
    int success_0 = 0, success_1 = 0, success_2 = 0, success_3 = 0, success_4 = 0, success_5 = 0;
    
    for (int trial = 0; trial < total_trials; ++trial) {
        // Generate random message
        std::vector<uint8_t> test_msg(223);
        for (int i = 0; i < 223; ++i) test_msg[i] = d(gen);
        
        auto test_codeword = bch_255.encode(test_msg);
        
        // Test with 0, 1, 2, 3, 4, 5 errors
        for (int errors = 0; errors <= 5; ++errors) {
            auto noisy_codeword = test_codeword;
            std::vector<int> positions(255);
            for (int i = 0; i < 255; ++i) positions[i] = i;
            std::shuffle(positions.begin(), positions.end(), gen);
            
            for (int i = 0; i < errors; ++i) {
                noisy_codeword[positions[i]] ^= 1;
            }
            
            std::vector<uint8_t> test_decoded;
            bool test_ok = bch_255.decode(noisy_codeword, test_decoded);
            
            if (test_ok && test_decoded == test_msg) {
                switch (errors) {
                    case 0: success_0++; break;
                    case 1: success_1++; break;
                    case 2: success_2++; break;
                    case 3: success_3++; break;
                    case 4: success_4++; break;
                    case 5: success_5++; break;
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
    std::cout << "5 errors: " << (double)success_5 / total_trials * 100 << "%\n";
    
    return 0;
}
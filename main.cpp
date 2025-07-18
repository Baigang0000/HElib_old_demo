#include <helib/helib.h>
#include <vector>
#include <random>
#include <iostream>
#include <cassert>
#include <cmath>
#include "bch/bch.h"

// CKKS parameters
constexpr long CKKS_N = 8192;
constexpr double DELTA = 1000.0;
constexpr int TRIALS = 100;  // Reduced for larger messages
constexpr double NOISE_LEVEL = 150.0;  // Moderate noise level for good BCH performance

// Binary ring parameters
constexpr double B = 1000.0;  // Upper bound on coefficients in R
constexpr int lambda_B = 10;  // λ_B = ceil(log2(1000)) = 10
constexpr int BINARY_RING_DIM = lambda_B * CKKS_N;  // λ_B * n = 10 * 8192 = 81920

// BCH parameters for larger messages
constexpr int BCH_N = 255;  // Use BCH(255,223,4) for larger codewords
constexpr int BCH_K = 223;
constexpr int BCH_T = 4;

std::vector<uint8_t> random_message(size_t length) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::bernoulli_distribution d(0.5);
    std::vector<uint8_t> msg(length);
    for (size_t i = 0; i < length; ++i) msg[i] = d(gen);
    return msg;
}

std::vector<double> bits_to_slots(const std::vector<uint8_t>& bits) {
    std::vector<double> slots(bits.size());
    for (size_t i = 0; i < bits.size(); ++i)
        slots[i] = bits[i] ? DELTA : -DELTA;
    return slots;
}

std::vector<uint8_t> slots_to_bits(const std::vector<double>& slots) {
    std::vector<uint8_t> bits(slots.size());
    for (size_t i = 0; i < slots.size(); ++i) {
        // Use a threshold-based approach: if value is closer to DELTA than -DELTA, it's 1
        bits[i] = (slots[i] > 0) ? 1 : 0;
    }
    return bits;
}

void add_noise(std::vector<double>& slots, double noise_level=NOISE_LEVEL) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::normal_distribution<> dis(0.0, noise_level);  // Gaussian noise with std dev = noise_level
    for (auto& slot : slots) slot += dis(gen);
}

// Split large binary message into chunks that fit in CKKS slots
std::vector<std::vector<uint8_t>> split_message(const std::vector<uint8_t>& message, int chunk_size) {
    std::vector<std::vector<uint8_t>> chunks;
    for (size_t i = 0; i < message.size(); i += chunk_size) {
        std::vector<uint8_t> chunk;
        for (int j = 0; j < chunk_size && i + j < message.size(); ++j) {
            chunk.push_back(message[i + j]);
        }
        chunks.push_back(chunk);
    }
    return chunks;
}

// Combine chunks back into a single message
std::vector<uint8_t> combine_chunks(const std::vector<std::vector<uint8_t>>& chunks) {
    std::vector<uint8_t> combined;
    for (const auto& chunk : chunks) {
        combined.insert(combined.end(), chunk.begin(), chunk.end());
    }
    return combined;
}

int main() {
    std::cout << "=== BCH-Enhanced Binary CKKS Experiment ===\n";
    std::cout << "Binary ring dimension: " << BINARY_RING_DIM << " (λ_B * n = " << lambda_B << " * " << CKKS_N << ")\n";
    std::cout << "BCH parameters: (" << BCH_N << ", " << BCH_K << ", " << BCH_T << ")\n";
    std::cout << "Number of trials: " << TRIALS << "\n\n";

    BCH bch;
    int success = 0;
    std::vector<int> bit_error_hist(20, 0);
    int total_bit_errors = 0;
    int total_bits_processed = 0;

    // HElib CKKS context setup
    helib::Context context = helib::ContextBuilder<helib::CKKS>()
        .m(CKKS_N)
        .bits(119)
        .precision(20)
        .build();
    
    // Generate keys
    helib::SecKey secret_key(context);
    secret_key.GenSecKey();
    helib::addSome1DMatrices(secret_key);
    const helib::PubKey& public_key = secret_key;

    std::cout << "Running BCH-CKKS experiment...\n";

    for (int trial = 0; trial < TRIALS; ++trial) {
        if (trial % 10 == 0) {
            std::cout << "Progress: " << trial << "/" << TRIALS << std::endl;
        }
        
        try {
            // 1. Generate random binary message of size λ_B * n
            auto msg = random_message(BINARY_RING_DIM);
            
            // 2. Split message into BCH-encodable chunks
            auto msg_chunks = split_message(msg, BCH_K);
            std::vector<uint8_t> encoded_message;
            
            // 3. BCH encode each chunk
            for (const auto& chunk : msg_chunks) {
                // Pad chunk to exactly BCH_K bits if needed
                std::vector<uint8_t> padded_chunk = chunk;
                while (padded_chunk.size() < BCH_K) {
                    padded_chunk.push_back(0);
                }
                
                auto codeword = bch.encode(padded_chunk);
                encoded_message.insert(encoded_message.end(), codeword.begin(), codeword.end());
            }
            
            // 4. Split encoded message into CKKS-encodable chunks
            int ckks_slots = CKKS_N / 4;  // CKKS actually provides n/4 slots, not n/2
            auto ckks_chunks = split_message(encoded_message, ckks_slots);
            
            std::vector<uint8_t> decrypted_message;
            
            // 5. Encrypt/decrypt each CKKS chunk
            int ckks_chunk_index = 0;
            for (const auto& ckks_chunk : ckks_chunks) {
                // Map to CKKS slots
                auto slots = bits_to_slots(ckks_chunk);
                
                // Pad to full slot count if needed
                while (slots.size() < ckks_slots) {
                    slots.push_back(0.0);
                }
                
                // CKKS encode/encrypt
                helib::PtxtArray ptxt(context, slots);
                helib::Ctxt ctxt(public_key);
                ptxt.encrypt(ctxt);
                
                // Decrypt
                helib::PtxtArray decrypted(context);
                decrypted.decrypt(ctxt, secret_key);
                std::vector<double> decrypted_slots;
                decrypted.store(decrypted_slots);
                
                // Add noise (simulate CKKS error)
                add_noise(decrypted_slots);
                
                // Quantize back to bits
                auto recovered_bits = slots_to_bits(decrypted_slots);
                
                // Use all recovered bits (up to ckks_slots)
                if (recovered_bits.size() > ckks_slots) {
                    recovered_bits.resize(ckks_slots);
                }
                
                decrypted_message.insert(decrypted_message.end(), 
                                       recovered_bits.begin(), recovered_bits.end());
                ckks_chunk_index++;
                
            }
            
            // 6. Count bit errors before BCH decode
            int bit_errors = 0;
            for (size_t i = 0; i < encoded_message.size() && i < decrypted_message.size(); ++i) {
                if (decrypted_message[i] != encoded_message[i]) ++bit_errors;
            }
            total_bit_errors += bit_errors;
            total_bits_processed += std::min(encoded_message.size(), decrypted_message.size());
            
            if (bit_errors >= (int)bit_error_hist.size())
                bit_error_hist.resize(bit_errors+1, 0);
            bit_error_hist[bit_errors]++;
            
            // 7. BCH decode each chunk
            auto decrypted_chunks = split_message(decrypted_message, BCH_N);
            std::vector<uint8_t> decoded_message;
            bool decode_success = true;
            
            for (const auto& chunk : decrypted_chunks) {
                if (chunk.size() < BCH_N) continue;  // Skip incomplete chunks
                
                std::vector<uint8_t> decoded_chunk;
                bool ok = bch.decode(chunk, decoded_chunk);
                if (!ok) {
                    decode_success = false;
                    break;
                }
                decoded_message.insert(decoded_message.end(), 
                                     decoded_chunk.begin(), decoded_chunk.end());
            }
            
            // 8. Check if original message was recovered
            if (decode_success && decoded_message.size() >= msg.size()) {
                decoded_message.resize(msg.size());
                if (decoded_message == msg) success++;
            }
            
        } catch (const std::exception& e) {
            std::cout << "Error in trial " << trial << ": " << e.what() << std::endl;
            continue;
        }
    }
    
    std::cout << "\n=== Results ===" << std::endl;
    std::cout << "Success rate: " << (double)success / TRIALS * 100 << "%" << std::endl;
    std::cout << "Successful decodings: " << success << "/" << TRIALS << std::endl;
    std::cout << "Average bit error rate: " << (double)total_bit_errors / total_bits_processed * 100 << "%" << std::endl;
    std::cout << "Bit error histogram: ";
    for (size_t i = 0; i < bit_error_hist.size(); ++i) {
        if (bit_error_hist[i] > 0)
            std::cout << i << ":" << bit_error_hist[i] << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
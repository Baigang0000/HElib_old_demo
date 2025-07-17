#include <helib/helib.h>
#include <vector>
#include <random>
#include <iostream>
#include <cassert>
#include "bch/bch.h"

constexpr long CKKS_N = 8192;
constexpr double DELTA = (double)(1ULL << 40);
constexpr int TRIALS = 10000;
constexpr double NOISE_LEVEL = 3.3e5;

std::vector<uint8_t> random_message(size_t k) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::bernoulli_distribution d(0.5);
    std::vector<uint8_t> msg(k);
    for (size_t i = 0; i < k; ++i) msg[i] = d(gen);
    return msg;
}

std::vector<double> bits_to_slots(const std::vector<uint8_t>& bits) {
    std::vector<double> slots(bits.size());
    for (size_t i = 0; i < bits.size(); ++i)
        slots[i] = bits[i] ? DELTA/2 : -DELTA/2;
    return slots;
}

std::vector<uint8_t> slots_to_bits(const std::vector<double>& slots) {
    std::vector<uint8_t> bits(slots.size());
    for (size_t i = 0; i < slots.size(); ++i)
        bits[i] = (slots[i] > 0) ? 1 : 0;
    return bits;
}

void add_noise(std::vector<double>& slots, double noise_level=NOISE_LEVEL) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-noise_level, noise_level);
    for (auto& slot : slots) slot += dis(gen);
}

int main() {
    BCH bch;
    int success = 0;
    std::vector<int> bit_error_hist(10, 0);

    // HElib CKKS context setup
    helib::Context context = helib::ContextBuilder<helib::CKKS>()
        .m(CKKS_N)
        .bits(119)
        .precision(40)
        .build();
    helib::SecKey secret_key(context);
    secret_key.GenSecKey();
    const helib::PubKey& public_key = secret_key;
    helib::EncryptedArrayCx ea(context);

    for (int trial = 0; trial < TRIALS; ++trial) {
        // 1. Generate random message
        auto msg = random_message(BCH::k);
        // 2. BCH encode
        auto codeword = bch.encode(msg);
        // 3. Map to CKKS slots
        auto slots = bits_to_slots(codeword);
        // 4. CKKS encode/encrypt
        helib::PtxtArray ptxt(context, slots);
        helib::Ctxt ctxt(public_key);
        ptxt.encrypt(ctxt);
        // 5. Decrypt
        helib::PtxtArray decrypted(context);
        decrypted.decrypt(ctxt, secret_key);
        std::vector<double> decrypted_slots;
        decrypted.store(decrypted_slots);
        // 6. Add noise (simulate CKKS error)
        add_noise(decrypted_slots);
        // 7. Quantize
        auto recovered_bits = slots_to_bits(decrypted_slots);
        // 8. Count bit errors before BCH decode
        int bit_errors = 0;
        for (size_t i = 0; i < codeword.size(); ++i)
            if (recovered_bits[i] != codeword[i]) ++bit_errors;
        if (bit_errors >= (int)bit_error_hist.size())
            bit_error_hist.resize(bit_errors+1, 0);
        bit_error_hist[bit_errors]++;
        // 9. BCH decode
        std::vector<uint8_t> decoded;
        bool ok = bch.decode(recovered_bits, decoded);
        if (ok && decoded == msg) success++;
    }
    std::cout << "Success rate: " << (double)success / TRIALS << std::endl;
    std::cout << "Bit error histogram: ";
    for (size_t i = 0; i < bit_error_hist.size(); ++i) {
        if (bit_error_hist[i] > 0)
            std::cout << i << ":" << bit_error_hist[i] << " ";
    }
    std::cout << std::endl;
    return 0;
}
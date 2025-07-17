# BCH-Enhanced Binary CKKS Experiment

This project demonstrates a BCH(127,106,3) error-correcting code integrated with CKKS encryption (using HElib) for robust binary message recovery under noise.

## Features
- **BCH(127,106,3) encoding/decoding** in C++
- **CKKS encryption/decryption** using HElib
- **Noise simulation** to test error correction
- **Automated experiment**: statistics on error correction and decoding success

---

## Prerequisites
- **C++17** or newer
- **CMake** (>=3.10)
- **HElib** (tested with v2.2.1+)

### Install HElib
Follow [HElib's official instructions](https://github.com/homenc/HElib#installation) or:
```bash
sudo apt-get install libntl-dev libgmp-dev
git clone https://github.com/homenc/HElib.git
cd HElib && mkdir build && cd build
cmake .. -DPEDANTIC_BUILD=OFF
make -j4
sudo make install
```

---

## Build and Run

### 1. Build the project
```bash
mkdir build && cd build
cmake ..
make -j4
```

### 2. Run the BCH-CKKS experiment
```bash
./bch_ckks_experiment
```

### 3. Test BCH error correction separately
```bash
./test_bch
```

---

## Expected Results

### BCH-CKKS Experiment
The main experiment will show:
- **Success rate**: Percentage of messages successfully recovered after CKKS encryption/decryption and BCH error correction
- **Bit error histogram**: Distribution of bit errors before BCH decoding
- **Progress updates**: During the experiment

Example output:
```
Running BCH-CKKS experiment with 1000 trials...
BCH parameters: n=127, k=106, t=3
Progress: 0/1000
...
=== Results ===
Success rate: 56.9%
Successful decodings: 569/1000
Bit error histogram: 0:1000
```

### BCH Error Correction Test
The BCH test will show:
- **Individual test cases**: 0, 1, 2, 3, and 4 bit errors
- **Success rates**: For different numbers of errors

Example output:
```
Testing BCH(127,106,3) error correction
==========================================

Test 1: No errors
Decode success: 1, Message correct: 1

Test 2: 1 bit error
Decode success: 1, Message correct: 1

Test 3: 2 bit errors
Decode success: 1, Message correct: 1

Test 4: 3 bit errors
Decode success: 1, Message correct: 1

Test 5: 4 bit errors (should fail)
Decode success: 0, Message correct: 0

Success rates:
0 errors: 100%
1 error:  100%
2 errors: 100%
3 errors: 100%
4 errors: 0%
```

---

## How It Works

### 1. **BCH Encoding**
- Takes 106-bit message
- Encodes to 127-bit codeword using BCH(127,106,3)
- Can correct up to 3 bit errors

### 2. **CKKS Encryption**
- Maps binary bits to CKKS slots (0 → -Δ, 1 → +Δ)
- Encrypts using HElib CKKS scheme
- Simulates noise by adding random values

### 3. **Decryption and Error Correction**
- Decrypts CKKS ciphertext
- Quantizes back to binary bits
- Applies BCH decoding to correct errors
- Extracts original 106-bit message

### 4. **Statistics**
- Counts successful recoveries
- Tracks bit error distribution
- Reports overall success rate

---

## Files

- `main.cpp` - Main BCH-CKKS experiment
- `test_bch.cpp` - BCH error correction test
- `bch/bch.h` - BCH class declaration
- `bch/bch.cpp` - BCH(127,106,3) implementation
- `CMakeLists.txt` - Build configuration
- `README.md` - This file

---

## Troubleshooting

### Build Issues
- **HElib not found**: Install HElib following the prerequisites
- **Compilation errors**: Ensure C++17 support and all dependencies

### Runtime Issues
- **Low success rate**: Adjust noise level in `main.cpp`
- **BCH decode failures**: Check BCH implementation in `bch/bch.cpp`

---

## Theory

This implementation demonstrates the BCH-enhanced binary CKKS workflow:

1. **Binary message** → **BCH encode** → **CKKS encrypt** → **Noise** → **CKKS decrypt** → **BCH decode** → **Recovered message**

The BCH code provides error correction capability, allowing exact recovery of binary messages even when CKKS introduces small errors during encryption/decryption.

For more details, see the paper section on BCH Extension and the workflow diagram.

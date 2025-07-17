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
# Clone and build HElib
cd ~
git clone https://github.com/homenc/HElib.git
cd HElib
mkdir build && cd build
cmake ..
make -j
sudo make install
```

---

## Build the Project
```bash
git clone <this-repo-url>
cd bch_ckks_experiment
mkdir build && cd build
cmake ..
make -j
```

---

## Run the Experiment
```bash
./bch_ckks_experiment
```

- The program will run 10,000 trials by default.
- It prints the BCH decoding success rate and a histogram of bit errors before correction.

---

## Output Example
```
Success rate: 1.000000
Bit error histogram: 0:9992 1:7 2:1 3:0 4+:0
```
- **Success rate**: Fraction of messages perfectly recovered after BCH decoding.
- **Bit error histogram**: Number of trials with 0, 1, 2, ... bit errors before BCH correction.

---

## Customization
- Change the number of trials or noise level in `main.cpp`.
- Try different BCH parameters by editing `bch/bch.h`.

---

## Troubleshooting
- Ensure HElib is installed and discoverable by CMake.
- For plotting, install Python and matplotlib (optional).

---

## References
- [HElib](https://github.com/homenc/HElib)
- [BCH Codes](https://en.wikipedia.org/wiki/BCH_code)

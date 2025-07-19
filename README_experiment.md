# CKKS vs. Binary CKKS Benchmark Suite

This repository contains a **reproducible, end‑to‑end benchmark** that compares Standard CKKS (as implemented in HElib) with a simplified *Binary CKKS* variant.  It produces ready‑to‑embed figures and LaTeX tables for academic papers.

---

## 1  Quick Start (Ubuntu / WSL)

```bash
# Clone & enter
git clone https://github.com/Baigang0000/HElib_old_demo.git
cd HElib_old_demo

# Create Python venv for analysis (optional but recommended)
python3 -m venv .venv
source .venv/bin/activate
pip install --upgrade pip pandas matplotlib seaborn numpy jinja2

# Install system deps (HElib + Google Benchmark + GMP / NTL)
sudo apt-get update
sudo apt-get install -y build-essential cmake libgmp-dev libntl-dev libbenchmark-dev

# Build HElib core
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
cd ..

# Build the benchmark executable
cd benchmarks
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make ckks_comparison
```

---

## 2  Run Benchmarks

```bash
# Inside benchmarks/build
./bin/ckks_comparison \
    --benchmark_format=json \
    --benchmark_out=ckks_results.json
```

*Operations measured*: KeyGen and Multiplication for ring sizes 1024, 2048, 4096, 8192.

---

## 3  Analyse & Plot

```bash
cd ~/HElib_old_demo
python3 benchmarks/analyze_benchmark_results.py \
        benchmarks/build/ckks_results.json \
        --output-dir results
```

This writes:

* `results/ckks_performance_comparison.pdf` – 4‑panel runtime plot.
* `results/ckks_results_table.tex` – ready‐to‐\input LaTeX table.
* `results/ckks_results_flat.csv` – flattened raw data.

Embed in LaTeX:

```latex
\begin{figure}[t]
  \centering
  \includegraphics[width=\linewidth]{results/ckks_performance_comparison.pdf}
  \caption{Runtime comparison of Standard vs. Binary CKKS. Lower is better.}
\end{figure}

\input{results/ckks_results_table.tex}
```

---

## 4  Customising the Benchmark

* **Ring sizes / security bits** – edit `benchmarks/ckks_comparison.cpp`.
* **Memory stats** – add `--benchmark_memory_usage` when running.
* **Extra operations** – extend the `BM_…` functions in the same file.
* **BCH‑enhanced Binary CKKS** – duplicate the Binary sections and call the BCH APIs.

Re‑run `make ckks_comparison` each time you change the C++.

---

## 5  Troubleshooting

| Symptom                             | Fix                                                                        |
| ----------------------------------- | -------------------------------------------------------------------------- |
| `fatal: could not create work tree` | Ensure you have write permission in the current folder.                    |
| `undefined reference` in link step  | Confirm `simple_binary_ckks.cpp` is listed in `benchmarks/CMakeLists.txt`. |
| `ModuleNotFoundError: jinja2`       | `pip install jinja2` in the active venv.                                   |
| Missing Google Benchmark            | `sudo apt install libbenchmark-dev` or build from source.                  |

---

## 6  Directory Structure (key files)

```
HElib_old_demo/
├── src/                    # Standard HElib source
├── benchmarks/
│   ├── ckks_comparison.cpp # Benchmark driver (edit here)
│   ├── simple_binary_ckks.h/cpp  # Simplified Binary CKKS impl
│   └── build/              # Out‑of‑source CMake build dir
└── results/                # Auto‑generated figures & tables
```

---

## 7  Acknowledgements

* HElib – Shai Halevi & Victor Shoup
* Google Benchmark – Google Inc.

Feel free to open issues or pull requests for further improvements.

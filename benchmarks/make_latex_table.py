#!/usr/bin/env python3
import argparse, pandas as pd

def fmt(x):
    # one decimal with comma thousands
    return f"{x:,.1f}"

def main():
    p = argparse.ArgumentParser()
    p.add_argument("--input",    "-i", help="all_results.csv",    required=True)
    p.add_argument("--speedups", "-s", help="speedups.csv",       required=True)
    args = p.parse_args()

    df    = pd.read_csv(args.input)
    speed = pd.read_csv(args.speedups)

    # pivot raw times
    std = df[df.scheme=="Standard"].pivot(index="N", columns="op", values="time_ms")
    bin = df[df.scheme=="Binary" ] .pivot(index="N", columns="op", values="time_ms")
    spd = speed.pivot(index="N", columns="op", values="speedup")

    # define order and label mapping
    ops      = ["KeyGen","Enc","Dec","Add","Mul"]
    labels   = {"KeyGen":"KeyGen",
                "Enc":"Encrypt",
                "Dec":"Decrypt",
                "Add":"Add",
                "Mul":"Multiply"}

    # header
    print(r"\begin{table}[h]")
    print(r"\scriptsize")
    print(r"\centering")
    print(r"\caption{Performance Comparison: Standard CKKS vs. Binary-CKKS. Runtimes are in microseconds ($\mu$s).}")
    print(r"\label{tab:perf}")
    print(r"\begin{tabular}{l l r r r }")
    print(r"\toprule")
    print(r"\textbf{Ring Dim.} & \textbf{Operation} & \textbf{Standard CKKS} & \textbf{Binary CKKS} & \textbf{Speedup} \\")
    print(r"\cmidrule(r){1-1}")
    print(r"\cmidrule(lr){2-2}")
    print(r"\cmidrule(lr){3-3}")
    print(r"\cmidrule(lr){4-4}")
    print(r"\cmidrule(l){5-5}")

    # rows
    for N in sorted(std.index):
        for i,op in enumerate(ops):
            t_std = std.at[N,op]
            t_bin = bin.at[N,op]
            s     = spd.at[N,op]

            s_std = fmt(t_std)
            s_bin = fmt(t_bin)
            s_spd = f"{s:.2f}$\\times$"

            if op=="KeyGen":
                # multirow start
                print(rf"\multirow{{5}}{{*}}{{{N}}}")
                print(rf"  & {labels[op]:<8} & {s_std:>7}   & {s_bin:>7}     & {s_spd:<7} \\")
            else:
                print(rf"  & {labels[op]:<8} & {s_std:>7}   & {s_bin:>7}     & {s_spd:<7} \\")
        print(r"\midrule")

    # footer
    print(r"\bottomrule")
    print(r"\end{tabular}")
    print(r"\end{table}")

if __name__=="__main__":
    main()

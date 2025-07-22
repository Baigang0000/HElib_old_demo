#!/usr/bin/env python3
import json, os, argparse
import pandas as pd
import matplotlib.pyplot as plt

def parse_args():
    p = argparse.ArgumentParser()
    p.add_argument("--input",  "-i", required=True,
                   help="Google Benchmark JSON file")
    p.add_argument("--out_dir", "-o", default=".", 
                   help="Where to write CSVs and plot")
    return p.parse_args()

def main():
    args = parse_args()
    data = json.load(open(args.input, "r"))
    rows = []
    for b in data["benchmarks"]:
        # name looks like "BM_KeyGen_Standard/1024"
        name, *_ = b["name"].split()  # discard extras if any
        op_full, Nstr = name.split("/")
        N = int(Nstr)
        if "Standard" in op_full:
            scheme = "Standard"
            op     = op_full.replace("BM_","").replace("_Standard","")
        else:
            scheme = "Binary"
            op     = op_full.replace("BM_","").replace("_Binary","")
        # pick the real_time (ms)
        time_ms = b.get("real_time", b.get("cpu_time"))  # whichever is available
        rows.append({
            "N": N,
            "scheme": scheme,
            "op": op,
            "time_ms": time_ms
        })

    df = pd.DataFrame(rows)
    os.makedirs(args.out_dir, exist_ok=True)
    all_csv = os.path.join(args.out_dir, "all_results.csv")
    df.to_csv(all_csv, index=False)
    print(f"Wrote raw results to {all_csv}")

    # compute speedups
    speed_rows = []
    for op in df["op"].unique():
        df_op = df[df["op"] == op]
        std = df_op[df_op["scheme"]=="Standard"].set_index("N")
        bin = df_op[df_op["scheme"]=="Binary"].set_index("N")
        for N in sorted(set(std.index).intersection(bin.index)):
            t_std = std.loc[N, "time_ms"]
            t_bin = bin.loc[N, "time_ms"]
            speed = t_std / t_bin if t_bin>0 else float("nan")
            speed_rows.append({"N": N, "op": op, "speedup": speed})
    speed_df = pd.DataFrame(speed_rows)
    speed_csv = os.path.join(args.out_dir, "speedups.csv")
    speed_df.to_csv(speed_csv, index=False)
    print(f"Wrote speedups to {speed_csv}")

    # Markdown table
    pivot = speed_df.pivot(index="N", columns="op", values="speedup")
    print("\n## Speedup (Standard / Binary)\n")
    print(pivot.to_markdown(floatfmt=".2f"))
    print()

    # Plot
    plt.figure(figsize=(8,5))
    for op in speed_df["op"].unique():
        sub = speed_df[speed_df["op"]==op]
        plt.plot(sub["N"], sub["speedup"], marker="o", label=op)
    plt.xscale("log", base=2)
    plt.xlabel("Ring dimension N")
    plt.ylabel("Speedup (Std / Bin)")
    plt.title("Binary-CKKS vs Standard CKKS Speedup by N")
    plt.grid(True, which="both", ls="--", alpha=0.5)
    plt.legend()
    plot_path = os.path.join(args.out_dir, "speedup_vs_N.png")
    plt.tight_layout()
    plt.savefig(plot_path, dpi=200)
    print(f"Wrote speedup plot to {plot_path}")

if __name__ == "__main__":
    main()

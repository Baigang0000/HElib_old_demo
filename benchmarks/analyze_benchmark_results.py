#!/usr/bin/env python3
# ── analyze_benchmark_results.py ──
import json, argparse, pathlib, pandas as pd, matplotlib.pyplot as plt

def load_json(path):
    with open(path) as f: j = json.load(f)
    return pd.json_normalize(j["benchmarks"])

def split_cols(df):
    # Benchmark name format: BM_<Op>_<Scheme>/<RingDim>
    df[["bench", "ring"]] = df["name"].str.extract(r"BM_(.*)/(\d+)")
    df["ring"] = df["ring"].astype(int)
    df[["op", "scheme"]] = df["bench"].str.split("_", n=1, expand=True)
    return df

def build_summary(df):
    grp = df[["scheme","op","ring","real_time"]].groupby(["scheme","op","ring"]).mean().reset_index()
    std = grp.pivot_table(index=["op","ring"], columns="scheme", values="real_time")
    std["speedup"] = std["Standard"].div(std["Binary"])
    return std.reset_index()

def latex_table(df, out):
    cols = ["op","ring","Standard","Binary","speedup"]
    dft = df.copy(); dft["speedup"] = dft["speedup"].round(2)
    dft.to_latex(out, index=False, float_format="%.3f", columns=cols, caption="Runtime ($\\mu$s) and speed‑ups")

def plot(df, out):
    fig, ax = plt.subplots(figsize=(6,4))
    for ring, sub in df.groupby("ring"):
        ax.bar(sub["op"]+"-"+sub["scheme"].str[0], sub["real_time"], label=str(ring))
    ax.set_ylabel("Runtime (µs)")
    ax.set_yscale("log"); ax.set_title("CKKS vs Binary CKKS")
    plt.tight_layout(); plt.savefig(out)
    
def main():
    p = argparse.ArgumentParser()
    p.add_argument("json"); p.add_argument("--output-dir", default="results/")
    a = p.parse_args()
    out = pathlib.Path(a.output_dir); out.mkdir(exist_ok=True)
    df = split_cols(load_json(a.json))
    df.to_csv(out/"ckks_results_flat.csv", index=False)
    summary = build_summary(df)
    latex_table(summary, out/"ckks_results_table.tex")
    plot(df, out/"ckks_performance_comparison.pdf")
    print("✓ analysis written to", out)

if __name__ == "__main__":
    import matplotlib
    matplotlib.use("Agg")  # headless
    main()

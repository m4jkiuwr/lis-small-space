import os
import sys
import random
import math
import subprocess
import csv
from dataclasses import dataclass
from typing import List
from pathlib import Path
import matplotlib.pyplot as plt
import pandas as pd
from tqdm import tqdm

@dataclass
class TestResult:
    n: int
    s: int
    classic_time: float
    classic_mem: float
    param_time: float
    param_mem: float
    valid: bool

def generate_test(in_path: Path, n_range: tuple[int, int], max_a: int, seed: int):
    random.seed(seed + hash(in_path.name))
    n = random.randint(n_range[0], n_range[1])
    s = int(math.ceil(math.sqrt(n)))
    arr = [random.randint(0, max_a) for _ in range(n)]
    with open(in_path, "w") as infile:
        infile.write(f"{n} {s}\n")
        infile.write(" ".join(map(str, arr)) + "\n")

def generate(test_dir: Path, num_tests: int, n_range: tuple[int, int], max_a=2**31 - 1, seed: int = 42):
    os.makedirs(test_dir, exist_ok=True)
    for idx in tqdm(iterable=range(1, num_tests + 1), desc="Generating tests"):
        in_path = test_dir / f"{idx}.in"
        generate_test(in_path=in_path, n_range=n_range, max_a=int(max_a), seed=seed)

def _compile(prog: Path) -> Path:
    tgt_path = prog.with_suffix(".exe").absolute()
    if prog.exists():
        print(f"Compiling {prog}...")
        subprocess.run(
            ["g++", "-O3", "-std=c++20", "-fsized-deallocation", str(prog), "-static", "-o", str(tgt_path)],
            check=True,
        )
        return tgt_path
    else:
        raise FileNotFoundError(f"Couldn't find the program under path: {prog}")

def run_test(exe_path: Path, in_path: Path) -> TestResult:
    result = subprocess.run([str(exe_path), str(in_path)], capture_output=True, text=True, check=True)
    parts = result.stdout.strip().split(',')
    
    res = TestResult(
        n=int(parts[0]),
        s=int(parts[1]),
        classic_time=float(parts[2]),
        classic_mem=float(parts[3]),
        param_time=float(parts[4]),
        param_mem=float(parts[5]),
        valid=(parts[6] == "True")
    )
    
    print("=" * 80)
    print(f"Test {in_path.name} (n={res.n}, s={res.s}):")
    print(f"Classic ran: {res.classic_time:.4f}s, used {res.classic_mem:.2f}MB")
    print(f"Parametrised ran: {res.param_time:.4f}s, used {res.param_mem:.2f}MB")
    print("=" * 80)
    
    return res

def run_all(exe_path: Path, test_dir: Path) -> List[TestResult]:
    results = []
    test_files = sorted(
        [f for f in test_dir.glob("*.in") if f.stem.isdigit()],
        key=lambda x: int(x.stem),
    )

    for in_path in test_files:
        res = run_test(exe_path, in_path)
        results.append(res)

    return results

def plot_results(csv_path: str):
    df = pd.read_csv(csv_path)
    df = df.sort_values(by="n")

    # 1. Comparison Plot (Classic vs Param)
    fig1, (ax1, ax2) = plt.subplots(1, 2, figsize=(15, 5))

    ticks = [i * 2 * 10**7 for i in range(6)]
    labels = [f"${i*2}*10^7$" if i > 0 else "0" for i in range(6)]
    labels[-1] = "$10^8$"

    ax1.plot(df["n"], df["classic_time"], marker="o", linestyle="-", label="Classic Time", color="blue")
    ax1.plot(df["n"], df["param_time"], marker="x", linestyle="-", label="Param Time", color="orange")
    ax1.set_xlabel("Number of elements (n)")
    ax1.set_ylabel("Time (s)")
    ax1.set_title("Comparison: Time vs. n")
    ax1.set_xlim(0, 1e8)
    ax1.set_xticks(ticks)
    ax1.set_xticklabels(labels)
    ax1.legend()
    ax1.grid(True)

    ax2.plot(df["n"], df["classic_mem"], marker="o", linestyle="-", label="Classic Mem (MB)", color="blue")
    ax2.plot(df["n"], df["param_mem"], marker="x", linestyle="-", label="Param Mem (MB)", color="orange")
    ax2.set_xlabel("Number of elements (n)")
    ax2.set_ylabel("Memory (MB)")
    ax2.set_title("Comparison: Memory vs. n")
    ax2.set_xlim(0, 1e8)
    ax2.set_xticks(ticks)
    ax2.set_xticklabels(labels)
    ax2.legend()
    ax2.grid(True)

    fig1.tight_layout()
    fig1.savefig("plot_comparison.png")
    print("Comparison plot saved to plot_comparison.png")
    plt.close(fig1)

    # 2. Param Only Plot
    fig2, (ax3, ax4) = plt.subplots(1, 2, figsize=(15, 5))

    ax3.plot(df["n"], df["param_time"], marker="x", linestyle="-", label="Param Time", color="orange")
    ax3.set_xlabel("Number of elements (n)")
    ax3.set_ylabel("Time (s)")
    ax3.set_title("Parametrized-space: Time vs. n")
    ax3.set_xlim(0, 1e8)
    ax3.set_xticks(ticks)
    ax3.set_xticklabels(labels)
    ax3.legend()
    ax3.grid(True)

    ax4.plot(df["n"], df["param_mem"], marker="x", linestyle="-", label="Param Mem (MB)", color="orange")
    ax4.set_xlabel("Number of elements (n)")
    ax4.set_ylabel("Memory (MB)")
    ax4.set_title("Parametrized-space: Memory vs. n")
    ax4.set_xlim(0, 1e8)
    ax4.set_xticks(ticks)
    ax4.set_xticklabels(labels)
    ax4.legend()
    ax4.grid(True)

    fig2.tight_layout()
    fig2.savefig("plot_param.png")
    print("Param plot saved to plot_param.png")
    plt.close(fig2)

if __name__ == "__main__":
    small_dir = Path("small_tests")
    medium_dir = Path("medium_tests")
    large_dir = Path("large_tests")

    print("Generating small tests...")
    generate(test_dir=small_dir, num_tests=10, n_range=(1, 100000), seed=42)

    print("Generating medium tests...")
    generate(test_dir=medium_dir, num_tests=80, n_range=(100000, 10000000), seed=4242)

    print("Generating large tests...")
    generate(test_dir=large_dir, num_tests=10, n_range=(10000000, 100000000), seed=424242)

    exe_path = _compile(prog=Path("test_lis.cpp"))

    all_results = []
    for d in [small_dir, medium_dir, large_dir]:
        print(f"Running tests in {d}...")
        all_results.extend(run_all(exe_path=exe_path, test_dir=d))

    csv_path = "results.csv"
    with open(csv_path, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(
            [
                "n",
                "s",
                "classic_time",
                "classic_mem",
                "param_time",
                "param_mem",
                "valid",
            ]
        )
        for r in all_results:
            writer.writerow(
                [
                    r.n,
                    r.s,
                    r.classic_time,
                    r.classic_mem,
                    r.param_time,
                    r.param_mem,
                    r.valid,
                ]
            )
    print(f"Results saved to {csv_path}")

    plot_results(csv_path)
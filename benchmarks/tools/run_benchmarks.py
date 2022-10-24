# coding=utf8
#!/usr/bin/env python

import subprocess
import argparse
import os

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Script to run benchmark with appropriate name.')
    parser.add_argument('exe', help='Path to benchmark executable.')
    parser.add_argument('-n', help='Name of benchmarks output file.')
    parser.add_argument('-f', help='Filter')
    parser.add_argument('-r', help='Repetition')
    args = parser.parse_args()
    array = [
        args.exe,
        "--benchmark_out=benchmark_" + os.path.basename(args.exe) + ".json",
    ]
    if args.f:
        array.append("--benchmark_filter=" + args.f)
    if args.r:
        array.append("--benchmark_repetitions=" + args.r)
        array.append("--benchmark_report_aggregates_only=true")

    subprocess.Popen(array, shell=True)
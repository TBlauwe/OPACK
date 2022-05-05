#!/usr/bin/env python

import subprocess
import argparse

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Script to run benchmark with appropriate name.')
    parser.add_argument('exe', help='Path to benchmark executable.')
    parser.add_argument('-n', help='Name of benchmarks output file.')
    args = parser.parse_args()
    subprocess.Popen([args.exe, "--benchmark_out=benchmark_" + args.n + ".json", "--benchmark_repetitions=10", "--benchmark_report_aggregates_only=true"])

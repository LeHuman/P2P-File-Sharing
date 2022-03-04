import glob
from os import chdir
import os
import subprocess
import argparse
import sys
import shutil
from time import sleep
import pandas as pd

def runTest(runs, interject, all2all):
    procs = []

    for i in range(runs):
        if i != interject:
            procs.append(subprocess.Popen(["Client.exe", f"-i {i}", "-c test_config.json", "-a" if all2all else ""]))

    # subprocess.Popen(f"wt Client.exe -i {interject}")

    for proc in procs:
        proc.wait()


def main():
    """Main Function"""
    runTest(int(sys.argv[1]), int(sys.argv[2]), len(sys.argv) > 3) # 29 n

main()
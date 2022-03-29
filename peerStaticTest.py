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

    all2all = "-a" if all2all else ""

    for i in range(runs):
        procs.append(subprocess.Popen(f"wt --title {i} Client.exe -i {i} -c test_config.json {all2all}"))
        sleep(0.03)

    for proc in procs:
        proc.wait()


def main():
    """Main Function"""
    runTest(int(sys.argv[1]), int(sys.argv[2]), len(sys.argv) > 3) # 29 n

main()
import glob
from os import chdir
import os
import subprocess
import argparse
import sys
import shutil
from time import sleep
import pandas as pd

def runTest(runs, mode, all2all, delay):
    procs = []

    all2all = "-a" if all2all == 1 else ""
    pushing = "-s" if (mode == 1 or mode == 3) else ""
    pulling = "-l" if (mode == 2 or mode == 3) else ""

    for i in range(runs):
        procs.append(subprocess.Popen(f"wt --title {i} Client.exe -i {i} -c test_config.json -r {delay} {all2all} {pushing} {pulling}"))
        sleep(0.03)

    for proc in procs:
        proc.wait()


def main():
    """Main Function"""
    
    shutil.copyfile("C:/Github/P2P-File-Sharing/out/build/x64-Debug/Client/Client.exe", "Client.exe")
    
    runTest(int(sys.argv[1]), int(sys.argv[2]), int(sys.argv[3]), int(sys.argv[4])) # 29 [1,2,3] [0,1] x>0

main()
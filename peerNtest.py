import glob
from os import chdir
import os
import subprocess
import argparse
import sys
import shutil
from time import sleep
import pandas as pd


def clean():
    for clean_up in glob.glob(os.getcwd() + "\*"):
        if not clean_up.endswith("Server.exe") and not clean_up.endswith("TestPeer.exe"):
            print(clean_up)
            try:
                os.remove(clean_up)
            except:
                shutil.rmtree(clean_up, ignore_errors=True)


def runTest(runs):
    procs = []

    for i in range(int(runs)):
        procs.append(subprocess.Popen(["TestPeer.exe", f"-i {i}"]))

    srv = subprocess.Popen("Server.exe")

    c = 0

    sleep(20)

    while c != len(procs):
        c = 0
        sleep(1)
        for root, dirs, files in os.walk("."):
            for file in files:
                c += os.path.splitext(file)[1] == ".done"
            break

    srv.kill()

    for proc in procs:
        proc.wait()


def getAvg(csv):
    column_sums = None
    with open(csv) as file:
        nums = file.readlines()[0].strip(",").split(",")
        nums = [num for num in map(int, nums)]
        s = sum(nums)
        return s / len(nums)


def readCSVs():
    avg = 0
    c = 0

    for csv in glob.glob(os.getcwd() + "\csvs\*"):
        avg += getAvg(csv)
        c += 1

    return avg / c


def main():
    """Main Function"""

    peers = []
    avgs = []

    clean()

    for i in list(range(3, 100)) + list(range(100, 500, 100)):
        runTest(i)
        peers.append(i)
        avgs.append(readCSVs())
        clean()
        pd.DataFrame({"Peers" : peers, "Avg micros" : avgs}).to_excel("Average Runtimes.xlsx")

main()
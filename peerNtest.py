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
        if not clean_up.endswith("test_config.json") and not clean_up.endswith("TestPeer.exe"):
            print(clean_up)
            try:
                os.remove(clean_up)
            except:
                shutil.rmtree(clean_up, ignore_errors=True)


def getAvg(csv):
    with open(csv) as file:
        nums = file.readlines()[0].strip(",").split(",")
        nums = [num for num in map(int, nums)]
        s = sum(nums)
        return s / len(nums)


def readCSVs():
    avg = 0
    c = 0

    for csv in glob.glob(os.getcwd() + "\csvs\*"):
        try:
            avg += getAvg(csv)
            c += 1
        except IndexError:  # File is empty?
            pass

    return avg / c


def runTest(maxID, active, all2all):
    procs = []

    for i in range(maxID - active):
        procs.append(subprocess.Popen(["TestPeer.exe", f"-i {i}", "-a" if all2all else ""]))

    for i in range(maxID - active, maxID + 1):
        procs.append(subprocess.Popen(["TestPeer.exe", f"-i {i}", "-e", "-a" if all2all else ""]))

    done = 0

    sleep(20)

    while done != len(procs):
        done = 0
        sleep(1)
        for _, _, files in os.walk("."):
            for file in files:
                done += os.path.splitext(file)[1] == ".done"
            break

    open("finish", "w").close()

    for proc in procs:
        proc.wait()


def main():
    """Main Function"""

    peers = []
    avgs = []

    clean()

    for i in range(0, 30):
        runTest(i, 29 - i, False)  # 29 1-29
        peers.append(i)
        avgs.append(readCSVs())
        clean()
        pd.DataFrame({"Active Peers": peers, "Avg micros": avgs}).to_excel("Average Runtimes Linear.xlsx")

    for i in range(0, 30):
        runTest(i, 29 - i, True)  # 29 1-29
        peers.append(i)
        avgs.append(readCSVs())
        clean()
        pd.DataFrame({"Active Peers": peers, "Avg micros": avgs}).to_excel("Average Runtimes All2All.xlsx")


main()

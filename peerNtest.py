import glob
from os import chdir
import os
import subprocess
import shutil
from time import sleep
import pandas as pd


def clean():
    for clean_up in glob.glob(os.getcwd() + "\*"):
        if not clean_up.endswith("test_config.json") and not clean_up.endswith("TestPeer.exe") and not clean_up.endswith(".xlsx"):
            print(clean_up)
            try:
                os.remove(clean_up)
            except:
                shutil.rmtree(clean_up, ignore_errors=True)


def getAvg(csv):
    with open(csv) as file:
        nums = file.readlines()[0].strip(",").split(",")
        nums = [num for num in map(float, nums)]
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

    if c == 0:
        return -1

    return avg / c


def runTest(maxID, active, pushORPull, all2all, delay):
    procs = []

    all2all = "-a" if all2all == 1 else ""
    pushing = "-s" if pushORPull else ""
    pulling = "-l" if not pushORPull else ""

    for i in range(maxID - active):
        procs.append(subprocess.Popen(f"TestPeer.exe -i {i} -c test_config.json -r {delay} {all2all} {pushing} {pulling}"))

    for i in range(maxID - active, maxID + 1):
        procs.append(subprocess.Popen(f"TestPeer.exe -i {i} -e -c test_config.json -r {delay} {all2all} {pushing} {pulling}"))

    sleep(1.5)

    open("start", "w").close()

    sleep(20)

    done = 0

    while done < len(procs):
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

    shutil.copyfile("C:/Github/P2P-File-Sharing/out/build/x64-Debug/TestPeer/TestPeer.exe", "TestPeer.exe")

    peers = []
    avgs = []

    clean()

    for i in range(27, -1, -1):
        runTest(28, 27 - i, True, True, 0)
        peers.append(28 - i)
        avgs.append(readCSVs())
        clean()
        pd.DataFrame({"Active Peers": peers, "Avg invalid": avgs}).to_excel("Average Runtimes Pushing All2All.xlsx")

    peers = []
    avgs = []

    for i in range(27, -1, -1):
        runTest(28, 27 - i, True, False, 0)
        peers.append(28 - i)
        avgs.append(readCSVs())
        clean()
        pd.DataFrame({"Active Peers": peers, "Avg invalid": avgs}).to_excel("Average Runtimes Pushing Linear.xlsx")

    for delay in [5,10,30]:
        peers = []
        avgs = []
        for i in range(27, -1, -1):
            runTest(28, 27 - i, False, True, delay)
            peers.append(28 - i)
            avgs.append(readCSVs())
            clean()
            pd.DataFrame({"Active Peers": peers, "Avg invalid": avgs}).to_excel(f"Average Runtimes Pulling All2All {delay}.xlsx")

    for delay in [5,10,30]:
        peers = []
        avgs = []
        for i in range(27, -1, -1):
            runTest(28, 27 - i, False, False, delay)
            peers.append(28 - i)
            avgs.append(readCSVs())
            clean()
            pd.DataFrame({"Active Peers": peers, "Avg invalid": avgs}).to_excel("Average Runtimes Pulling Linear {delay}.xlsx")


main()
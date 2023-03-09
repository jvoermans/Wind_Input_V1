import time
import os

from decoder import GNSS_Packet, Data_Message, decode_file, local_timebase_s

from pathlib import Path

from icecream import ic

import matplotlib.pyplot as plt

print("***** Put the interpreter in UTC, to make sure no TZ issues")
os.environ["TZ"] = "UTC"
time.tzset()

# ------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------

# what folder to decript
basepath = Path("../data/")

# go through all data files
for crrt_file in basepath.glob("*.dat"):
    ic(crrt_file)

    # actually decode
    kind, data = decode_file(crrt_file)
    ic(kind)
    ic(data)

    # a bit of plotting in case this is a data file;
    # plot the 2 most important things: accD and press1
    if kind == "data_file":
        sample_rate_hz = 20

        plt.figure()
        plt.plot(local_timebase_s, data.accD)
        plt.xlabel("s since start")
        plt.ylabel("accD [m/s2]")

        plt.figure()
        plt.plot(local_timebase_s, data.press1)
        plt.xlabel("s since start")
        plt.ylabel("press1 [hPa]")

        plt.show()

        # accD

        # press1

    print("")


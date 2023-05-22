import time
import os

from decoder import decode_file, local_timebase_s

from pathlib import Path

from icecream import ic

import matplotlib.pyplot as plt

import datetime

print("***** Put the interpreter in UTC, to make sure no TZ issues")
os.environ["TZ"] = "UTC"
time.tzset()

# ------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------

# what folder to decript
basepath = Path("/home/jrmet/Downloads/2023_05_19_Data/2023_05_19_Final")

# go through all data files
for crrt_file in basepath.glob("*.dat"):
    with basepath / crrt_file as crrt_file:
        ic(crrt_file)

        # actually decode
        kind, data = decode_file(crrt_file)
        ic(kind)
        ic(data)

        # a bit of plotting in case this is a data file;
        # plot the 2 most important things: accD and press1
        if kind == "data_file":
            # use the fix_end, and remove the typical duration of the measurement, to get typical time start
            # if need to be really precise, time start is the closest to minute % 10 == 0, that is just before the following
            time_start = data.fix_end.datetime_fix - datetime.timedelta(minutes=6)

            plt.figure()
            plt.plot(local_timebase_s, data.accD)
            plt.xlabel(f"seconds since {time_start}")
            plt.ylabel("accD [m/s2]")
            plt.tight_layout()
            plt.savefig("accD.png")

            plt.figure()
            plt.plot(local_timebase_s, data.press1)
            plt.xlabel(f"seconds since {time_start}")
            plt.ylabel("press1 [hPa]")
            plt.tight_layout()
            plt.savefig("press1.png")

            plt.figure()
            plt.plot(local_timebase_s, data.press2)
            plt.xlabel(f"seconds since {time_start}")
            plt.ylabel("press2 [hPa]")
            plt.tight_layout()
            plt.savefig("press2.png")

            plt.show()

        print("")


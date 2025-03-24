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

print("")
print("--- NOTE ---")
print("here we extract only a few fields, more data are available if needed")
print("see the decoder.py Data_Message content for more information about fields")
print("")

# ------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------

# what folder to decript
basepath = Path("/home/jeanr/Downloads/Presure_w220/Pressure_w230/")

# go through all data files
for crrt_file in sorted(list(basepath.glob("*.dat"))):
    crrt_file = basepath / crrt_file
    ic(crrt_file)
    crrt_file_csv = Path(str(crrt_file)[0:-3] + "csv")
    ic(crrt_file_csv)

    bytesize = crrt_file.stat().st_size
    if bytesize < 100e3:
        print("small file, likely empty of corrupted, skip")
        continue

    # actually decode
    kind, data = decode_file(crrt_file)
    ic(kind)
    ic(data)

    # a bit of plotting in case this is a data file;
    # plot the 2 most important things: accD and press1
    if kind == "data_file":
        # use the fix_end, and remove the typical duration of the measurement, to get typical time start
        time_start = data.fix_end.datetime_fix - datetime.timedelta(minutes=6)
        # if need to be really precise, time start is the closest preceding minute % 10 == 0, due to GNSS acquisition time
        # since the duration of a measurement is 6 mins, and the times when measurements start is on UTC multiples of 10 minutes
        time_start = time_start.replace(minute=time_start.minute - (time_start.minute % 10), second = 0)
        ic(time_start)

        if False:
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

        # NOTE: example of how to dump as csv; note that more information are available in the python objects
        # (can be added to the CSVs if needed)
        # see the decoder.py file for more information, the fields are:
        # 
        # @dataclass
        # class Data_Message:
        #     fix_start: GNSS_Packet
        #     fix_end : GNSS_Packet
        #     accX: np.ndarray
        #     accY: np.ndarray
        #     accZ: np.ndarray
        #     accD: np.ndarray
        #     pitch: np.ndarray
        #     roll: np.ndarray
        #     press1: np.ndarray
        #     press2: np.ndarray
        #     loopidx: int
        with open(crrt_file_csv, "w") as fh:
            fh.write("timestamp, accD, press1, press2\n")
            for crrt_local_timebase_s, crrt_accD, crrt_press1, crrt_press2 in zip(local_timebase_s, data.accD, data.press1, data.press2):
                crrt_time = (time_start + datetime.timedelta(seconds=crrt_local_timebase_s)).isoformat()
                fh.write(f"{crrt_time},{crrt_accD},{crrt_press1},{crrt_press2}\n")

    print("")



# ------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------
# preamble

import time
import os

from pathlib import Path

# ------------------------------------------------------------------------------------------
print("***** Put the interpreter in UTC, to make sure no TZ issues")
os.environ["TZ"] = "UTC"
time.tzset()

from icecream import ic

import datetime
import pytz

# pytz is likely more flexible
utc_timezone = pytz.timezone("UTC")
# there are also some datetime native timezones
datetime_utc_timezone = datetime.timezone.utc

ic.configureOutput(prefix="", outputFunction=print)

# ------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------
# "magic" semaphore strings to look for in files

# for files of kind data

string_start_gnss_start = b"\n\nDATA\n\n\n\nGNSS_start\n\n"
string_start_gnss_done_end_gnss_start = b"\n\nGNSS_start_done\n\n\n\nGNSS_end\n\n"
string_end_gnss_done = b"\n\nGNSS_end_done\n\n\n\n"

string_start_gnss_start_ascii = b"GNSS_start_string\n\n"
string_start_gnss_done_ascii = b"\n\nGNSS_start_string_done\n\n\n\n"
string_end_gnss_start_ascii = b"GNSS_end_string\n\n"
string_end_gnss_done_ascii = b"\n\nGNSS_end_string_done\n\n\n\n"

string_accx_start = b"accX_array\n\n"
string_accx_end = b"\n\naccX_array_done\n\n\n\n"

string_accy_start = b"accY_array\n\n"
string_accy_end = b"\n\naccY_array_done\n\n\n\n"

string_accz_start = b"accZ_array\n\n"
string_accz_end = b"\n\naccZ_array_done\n\n\n\n"

string_accd_start = b"accD_array\n\n"
string_accd_end = b"\n\naccD_array_done\n\n\n\n"

string_press1_start = b"press1_array\n\n"
string_press1_end = b"\n\npress1_array_done\n\n\n\n"

string_press2_start = b"press2_array\n\n"
string_press2_end = b"\n\npress2_array_done\n\n\n\n"

string_file_end = b"DATA-done\n\n"

list_expected_strings_datafile = [
    string_start_gnss_start,
    string_start_gnss_done_end_gnss_start,
    string_end_gnss_done,
    string_start_gnss_start_ascii,
    string_start_gnss_done_ascii,
    string_end_gnss_start_ascii,
    string_end_gnss_done_ascii,
    string_accx_start,
    string_accx_end,
    string_accy_start,
    string_accy_end,
    string_accz_start,
    string_accz_end,
    string_accd_start,
    string_accd_end,
    string_press1_start,
    string_press1_end,
    string_press2_start,
    string_press2_end,
    string_file_end,
]

# for files of kind boot

string_boot_gnss_binary_start = b"\n\nBOOT\n\n\n\nGNSS_binary\n\n"
string_boot_gnss_binary_done_gnss_ascii_start = b"\n\nGNSS_binary_done\n\n\n\nGNSS_string\n\n"
string_boot_gnss_ascii_done_file_done = b"\n\nGNSS_string_done\n\n\n\nBOOT_done\n\n"

list_expected_strings_bootfile = [
    string_boot_gnss_binary_start,
    string_boot_gnss_binary_done_gnss_ascii_start,
    string_boot_gnss_ascii_done_file_done,
]

# ------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------
# for now, decode as a script, will make into function soon

# ------------------------------------------------------------------------------------------
# file bootfile kind
file_to_decode = Path("./../data/2023-03-08-19-15.dat")

with open(file_to_decode, "br") as fh:
    data = fh.read()

for crrt_bstr in list_expected_strings_bootfile:
    assert crrt_bstr in data

assert data.find(string_boot_gnss_binary_start) == 0

idx_start_gnss_binary = len(string_boot_gnss_binary_start)
idx_end_gnss_binary = data.find(string_boot_gnss_binary_done_gnss_ascii_start)
string_boot_gnss_raw = data[idx_start_gnss_binary:idx_end_gnss_binary]

idx_start_gnss_ascii = idx_end_gnss_binary + len(string_boot_gnss_binary_done_gnss_ascii_start)
idx_end_gnss_ascii = data.find(string_boot_gnss_ascii_done_file_done)
string_boot_gnss_ascii = data[idx_start_gnss_ascii:idx_end_gnss_ascii]

# ------------------------------------------------------------------------------------------
# file datafile kind
file_to_decode = Path("./../data/2023-03-08-19-21.dat")


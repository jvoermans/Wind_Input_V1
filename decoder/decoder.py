
# ------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------
# preamble

import time
import os

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

string_start_gnss_start = r"\n\nDATA\n\n\n\nGNSS_start\n\n"
string_start_gnss_done_end_gnss_start = r"\n\nGNSS_start_done\n\n\n\nGNSS_end\n\n"
string_end_gnss_done = r"\n\nGNSS_end_done\n\n\n\n"

string_start_gnss_start_ascii = r"GNSS_start_string\n\n"
string_start_gnss_done_ascii = r"\n\nGNSS_start_string_done\n\n\n\n"
string_end_gnss_start_ascii = r"GNSS_end_string\n\n"
string_end_gnss_done_ascii = r"\n\nGNSS_end_string_done\n\n\n\n"

string_accx_start = r"accX_array\n\n"
string_accx_end = r"\n\naccX_array_done\n\n\n\n"

string_accy_start = r"accY_array\n\n"
string_accy_end = r"\n\naccY_array_done\n\n\n\n"

string_accz_start = r"accZ_array\n\n"
string_accz_end = r"\n\naccZ_array_done\n\n\n\n"

string_accd_start = r"accD_array\n\n"
string_accd_end = r"\n\naccD_array_done\n\n\n\n"

string_press1_start = r"press1_array\n\n"
string_press1_end = r"\n\npress1_array_done\n\n\n\n"

string_press2_start = r"press2_array\n\n"
string_press2_end = r"\n\npress2_array_done\n\n\n\n"

string_file_end = r"DATA-done\n\n"

list_expected_strings = [
    string_start_gnss_start,
]



# ------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------
# preamble

import time
import os

import struct

from dataclasses import dataclass

# use pprint for python above 3.10, prettyprinter otherwise
# import pprint
import prettyprinter as pp

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

pp.install_extras()

# ------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------
# key structs

@dataclass
class GNSS_Packet:
    datetime_fix: datetime.datetime
    latitude: float
    longitude: float


@dataclass
class Data_Message:
    fix_start: GNSS_Packet
    fix_end : GNSS_Packet

# ------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------
# "magic" semaphore strings to look for in files

# for files of kind data

string_data_start_gnss_start = b"\n\nDATA\n\n\n\nGNSS_start\n\n"
string_data_start_gnss_done_end_gnss_start = b"\n\nGNSS_start_done\n\n\n\nGNSS_end\n\n"
string_data_end_gnss_done = b"\n\nGNSS_end_done\n\n\n\n"

string_data_start_gnss_start_ascii = b"GNSS_start_string\n\n"
string_data_start_gnss_done_ascii = b"\n\nGNSS_start_string_done\n\n\n\n"
string_data_end_gnss_start_ascii = b"GNSS_end_string\n\n"
string_data_end_gnss_done_ascii = b"\n\nGNSS_end_string_done\n\n\n\n"

string_data_accx_start = b"accX_array\n\n"
string_data_accx_end = b"\n\naccX_array_done\n\n\n\n"

string_data_accy_start = b"accY_array\n\n"
string_data_accy_end = b"\n\naccY_array_done\n\n\n\n"

string_data_accz_start = b"accZ_array\n\n"
string_data_accz_end = b"\n\naccZ_array_done\n\n\n\n"

string_data_accd_start = b"accD_array\n\n"
string_data_accd_end = b"\n\naccD_array_done\n\n\n\n"

string_data_press1_start = b"press1_array\n\n"
string_data_press1_end = b"\n\npress1_array_done\n\n\n\n"

string_data_press2_start = b"press2_array\n\n"
string_data_press2_end = b"\n\npress2_array_done\n\n\n\n"

string_data_file_end = b"DATA-done\n\n"

list_expected_strings_datafile = [
    string_data_start_gnss_start,
    string_data_start_gnss_done_end_gnss_start,
    string_data_end_gnss_done,
    string_data_start_gnss_start_ascii,
    string_data_start_gnss_done_ascii,
    string_data_end_gnss_start_ascii,
    string_data_end_gnss_done_ascii,
    string_data_accx_start,
    string_data_accx_end,
    string_data_accy_start,
    string_data_accy_end,
    string_data_accz_start,
    string_data_accz_end,
    string_data_accd_start,
    string_data_accd_end,
    string_data_press1_start,
    string_data_press1_end,
    string_data_press2_start,
    string_data_press2_end,
    string_data_file_end,
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
# utility functions


def identify_file_kind(path_to_file: Path) -> str:
    with open(path_to_file, "br") as fh:
        data = fh.read()

    is_start_file = all([crrt_string in data for crrt_string in list_expected_strings_bootfile])
    is_data_file = all([crrt_string in data for crrt_string in list_expected_strings_datafile])

    assert (is_start_file or is_data_file)
    assert ((not is_start_file) or (not is_data_file))

    if is_start_file:
        return "start_file"
    if is_data_file:
        return "data_file"

    raise RuntimeError("should be a start or a data file")


def decode_gnss_ascii_string(gnss_ascii_string: str) -> GNSS_Packet:
    gnss_ascii_fields = gnss_ascii_string.replace(b"\n", b"").split(b"\r")

    assert gnss_ascii_fields[6] in [b"N", b"S"]
    assert gnss_ascii_fields[8] in [b"E", b"W"]

    if gnss_ascii_fields[6] == b"N":
        sign_lat = 1.0
    else:
        sign_lat = -1.0

    if gnss_ascii_fields[8] == b"E":
        sign_lon = 1.0
    else:
        sign_lon = -1.0

    result_fix = GNSS_Packet(
        datetime_fix = datetime.datetime(
            int(gnss_ascii_fields[0]),
            int(gnss_ascii_fields[1]),
            int(gnss_ascii_fields[2]),
            int(gnss_ascii_fields[3]),
            int(gnss_ascii_fields[4]),
            0, 0, tzinfo=datetime.timezone.utc
        ),
        latitude = sign_lat * float(gnss_ascii_fields[5]) / 1e7,
        longitude = sign_lon * float(gnss_ascii_fields[7]) / 1e7,
    )

    return result_fix


def decode_start_file(path_to_file: Path) -> GNSS_Packet:
    with open(path_to_file, "br") as fh:
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
    gnss_fix = decode_gnss_ascii_string(string_boot_gnss_ascii)

    return gnss_fix


def decode_file(path_to_file: Path) -> (str, type):
    kind = identify_file_kind(path_to_file)

    if kind == "start_file":
        gnss_fix = decode_start_file(path_to_file)
        return ("start_file", gnss_fix)

    if kind == "data_file":
        pass
        return ("data_file", None)

# ------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------
# for now, decode as a script, will make into function soon

# ------------------------------------------------------------------------------------------
# file bootfile kind
file_to_decode_start = Path("./../data/2023-03-08-19-15.dat")

(kind, data) = decode_file(file_to_decode_start)
ic(kind)
ic(data)

# ------------------------------------------------------------------------------------------
# file datafile kind
file_to_decode_data = Path("./../data/2023-03-08-19-21.dat")

with open(file_to_decode_data, "br") as fh:
    data = fh.read()

for crrt_bstr in list_expected_strings_datafile:
    assert crrt_bstr in data

assert data.find(string_data_start_gnss_start) == 0

idx_start_gnss_start = data.find(string_data_start_gnss_start_ascii) + len(string_data_start_gnss_start_ascii)
idx_start_gnss_end = data.find(string_data_start_gnss_done_ascii)
string_data_start_gnss = data[idx_start_gnss_start: idx_start_gnss_end]
gnss_fix_start = decode_gnss_ascii_string(string_data_start_gnss)

idx_end_gnss_start = data.find(string_data_end_gnss_start_ascii) + len(string_data_end_gnss_start_ascii)
idx_end_gnss_end = data.find(string_data_end_gnss_done_ascii)
string_data_end_gnss = data[idx_end_gnss_start: idx_end_gnss_end]
gnss_fix_end = decode_gnss_ascii_string(string_data_end_gnss)

idx_accX_start =
idx_accX_end =

idx_accY_start =
idx_accY_end =

idx_accZ_start =
idx_accZ_end =

idx_accD_start =
idx_accD_end =

idx_press1_start =
idx_press1_end =

idx_press2_start =
idx_press2_end =

result = Data_Message(
    fix_start=gnss_fix_start,
    fix_end=gnss_fix_end
)

pp.pprint(result)


(kind, data) = decode_file(file_to_decode_data)
ic(kind)
ic(data)


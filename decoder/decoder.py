# ------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------
# preamble

import time
import os

import struct

from dataclasses import dataclass

# use pprint for python above 3.10, prettyprinter otherwise
# import pprint
# import prettyprinter as pp

import numpy as np

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

# pp.install_extras()

# ------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------
# key structs

@dataclass
class GNSS_Packet:
    datetime_fix: datetime.datetime
    latitude: float
    longitude: float
    valid: bool


@dataclass
class Data_Message:
    fix_start: GNSS_Packet
    fix_end : GNSS_Packet
    accX: np.ndarray
    accY: np.ndarray
    accZ: np.ndarray
    accD: np.ndarray
    pitch: np.ndarray
    roll: np.ndarray
    press1: np.ndarray
    press2: np.ndarray
    loopidx: int

# ------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------
# "magic" semaphore strings, values, etc, to decode files

# for files of kind data

n_mins_logging = 6
n_samples_per_buffer = 20 * n_mins_logging * 60 + 1
np_data_length = 20 * n_mins_logging * 60

local_timebase_s = np.arange(0.0, np_data_length * 1.0 / 20.0, 1.0 / 20.0)

float_value_decode_uint16_t = 65000.0
float_value_decode_uint32_t = 4294900000.0

string_data_start_gnss_start = b"\n\nDATA\n\n\n\nGNSS_start\n\n"
string_data_start_gnss_done_end_gnss_start = b"\n\nGNSS_start_done\n\n\n\nGNSS_end\n\n"
string_data_end_gnss_done = b"\n\nGNSS_end_done\n\n\n\n"

string_loopidx_start = b"loopidx_start_string\n\n"
string_loopidx_done = b"\n\nloopidx_done\n\n\n\n"

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

string_data_pitch_start = b"pitch_array\n\n"
string_data_pitch_end = b"\n\npitch_array_done\n\n\n\n"

string_data_roll_start = b"roll_array\n\n"
string_data_roll_end = b"\n\nroll_array_done\n\n\n\n"

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
    string_loopidx_start,
    string_loopidx_done,
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
    string_data_pitch_start,
    string_data_pitch_end,
    string_data_roll_start,
    string_data_roll_end,
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

def get_string_start_idx(data_buffer: str, string: str) -> int:
    return data_buffer.find(string)


def get_string_startnext_idx(data_buffer: str, string: str) -> int:
    """Find the index of the start of the str following the first
    appearance of string in data_buffer."""
    return data_buffer.find(string) + len(string)


def get_buffer_inbetween_strings(data_buffer: str, string_start: str, string_end: str) -> str:
    """Find the part of the buffer in between string_start and string_end,
    ie between the end of string_start and the start of string_end."""
    idx_start = get_string_startnext_idx(data_buffer, string_start)
    idx_end = get_string_start_idx(data_buffer, string_end)
    return data_buffer[idx_start:idx_end]


def decode_uint_buffer_inbetween_strings(data_buffer: str, string_start: str, string_end: str, kind: str) -> list:
    """Decode the uint buffer from data_buffer in between string delimiters string_start and string_end.
    The kind is either H for uint16_t, or I for uint32_t. The length is the length of the data buffers."""

    if kind == "H":
        typesize = 2
    elif kind == "I":
        typesize = 4
    else:
        raise RuntimeError(f"Unknown typesize kind; expect H or I, got {kind}")

    data = get_buffer_inbetween_strings(data_buffer, string_start, string_end)
    assert len(data) == typesize * n_samples_per_buffer
    array_uint = struct.unpack('<' + n_samples_per_buffer * kind, data)

    # we always discard the last entry, which is always 0 and is not used (just "0-semaphore")
    array_uint = array_uint[:-1]

    return array_uint


def denormalize_uint_array(uint_array_in: list, kind: str, range_data: float, offset_data: float) -> np.ndarray:
    """Denormalize uint_array_in of kind (uint16_t: 'H', uint32_t: 'I'), with range and
    offset as given, into a np array of physical values."""

    if kind == "H":
        float_decode = float_value_decode_uint16_t
    elif kind == "I":
        float_decode = float_value_decode_uint32_t
    else:
        raise RuntimeError(f"Unknown typesize kind; expect H or I, got {kind}")

    return np.array(uint_array_in, dtype=np.float32) / float_decode * range_data - offset_data


def get_decoded_denormalized_array(data_buffer: str, string_start: str, string_end:str, kind: str, range_data: float, offset_data:float) -> np.ndarray:
    array_uint = decode_uint_buffer_inbetween_strings(data_buffer, string_start, string_end, kind)
    array_out = denormalize_uint_array(array_uint, kind, range_data, offset_data)
    return array_out


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

    assert gnss_ascii_fields[7] in [b"N", b"S", b"X"]
    assert gnss_ascii_fields[9] in [b"E", b"W", b"X"]

    if gnss_ascii_fields[7] == b"X" or gnss_ascii_fields[9] == b"X":
        result_fix = GNSS_Packet(
            datetime_fix = datetime.datetime(
                1970,
                1,
                1,
                0,
                0,
                0,
                0, tzinfo=datetime.timezone.utc
            ),
            latitude = 0.0,
            longitude = 0.0,
            valid = False,
        )
        return result_fix

    if gnss_ascii_fields[7] == b"N":
        sign_lat = 1.0
    else:
        sign_lat = -1.0

    if gnss_ascii_fields[9] == b"E":
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
            int(gnss_ascii_fields[5]),
            0, tzinfo=datetime.timezone.utc
        ),
        latitude = sign_lat * float(gnss_ascii_fields[6]) / 1e7,
        longitude = sign_lon * float(gnss_ascii_fields[8]) / 1e7,
        valid = True,
    )

    return result_fix


def decode_start_file(path_to_file: Path) -> GNSS_Packet:
    with open(path_to_file, "br") as fh:
        data = fh.read()

    for crrt_bstr in list_expected_strings_bootfile:
        assert crrt_bstr in data

    assert data.find(string_boot_gnss_binary_start) == 0

    # idx_start_gnss_binary = len(string_boot_gnss_binary_start)
    idx_end_gnss_binary = data.find(string_boot_gnss_binary_done_gnss_ascii_start)
    # string_boot_gnss_raw = data[idx_start_gnss_binary:idx_end_gnss_binary]

    idx_start_gnss_ascii = idx_end_gnss_binary + len(string_boot_gnss_binary_done_gnss_ascii_start)
    idx_end_gnss_ascii = data.find(string_boot_gnss_ascii_done_file_done)
    string_boot_gnss_ascii = data[idx_start_gnss_ascii:idx_end_gnss_ascii]
    gnss_fix = decode_gnss_ascii_string(string_boot_gnss_ascii)

    return gnss_fix


def decode_data_file(path_to_file:Path) -> Data_Message:
    with open(path_to_file, "br") as fh:
        data = fh.read()

    for crrt_bstr in list_expected_strings_datafile:
        assert crrt_bstr in data

    assert data.find(string_data_start_gnss_start) == 0

    idx_start_loopidx = data.find(string_loopidx_start) + len(string_loopidx_start)
    idx_end_loopidx = data.find(string_loopidx_done)
    loopidx = int(data[idx_start_loopidx:idx_end_loopidx])

    idx_start_gnss_start = data.find(string_data_start_gnss_start_ascii) + len(string_data_start_gnss_start_ascii)
    idx_start_gnss_end = data.find(string_data_start_gnss_done_ascii)
    string_data_start_gnss = data[idx_start_gnss_start: idx_start_gnss_end]
    gnss_fix_start = decode_gnss_ascii_string(string_data_start_gnss)

    idx_end_gnss_start = data.find(string_data_end_gnss_start_ascii) + len(string_data_end_gnss_start_ascii)
    idx_end_gnss_end = data.find(string_data_end_gnss_done_ascii)
    string_data_end_gnss = data[idx_end_gnss_start: idx_end_gnss_end]
    gnss_fix_end = decode_gnss_ascii_string(string_data_end_gnss)

    np_accX = get_decoded_denormalized_array(
        data,
        string_data_accx_start,
        string_data_accx_end,
        "H",
        4.0 * 9.81,
        2.0 * 9.81
    )

    np_accY = get_decoded_denormalized_array(
        data,
        string_data_accy_start,
        string_data_accy_end,
        "H",
        4.0 * 9.81,
        2.0 * 9.81
    )

    np_accZ = get_decoded_denormalized_array(
        data,
        string_data_accz_start,
        string_data_accz_end,
        "H",
        4.0 * 9.81,
        2.0 * 9.81
    )

    np_pitch = get_decoded_denormalized_array(
        data,
        string_data_pitch_start,
        string_data_pitch_end,
        "H",
        2.0 * 360.0,
        360.0
    )

    np_roll = get_decoded_denormalized_array(
        data,
        string_data_roll_start,
        string_data_roll_end,
        "H",
        2.0 * 360.0,
        360.0
    )

    np_accD = get_decoded_denormalized_array(
        data,
        string_data_accd_start,
        string_data_accd_end,
        "I",
        2.0 * 9.81,
        0.0
    )

    np_press1 = get_decoded_denormalized_array(
        data,
        string_data_press1_start,
        string_data_press1_end,
        "I",
        250.0,
        -850.0
    )

    np_press2 = get_decoded_denormalized_array(
        data,
        string_data_press2_start,
        string_data_press2_end,
        "I",
        250.0,
        -850.0
    )

    data_content = Data_Message(
        fix_start = gnss_fix_start,
        fix_end = gnss_fix_end,
        accX = np_accX,
        accY = np_accY,
        accZ = np_accZ,
        pitch = np_pitch,
        roll = np_roll,
        accD = np_accD,
        press1 = np_press1,
        press2 = np_press2,
        loopidx = loopidx
    )

    return data_content


def decode_file(path_to_file: Path) -> (str, type):
    kind = identify_file_kind(path_to_file)

    if kind == "start_file":
        gnss_fix = decode_start_file(path_to_file)
        return ("start_file", gnss_fix)

    if kind == "data_file":
        data_content = decode_data_file(path_to_file)
        return ("data_file", data_content)

# ------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------
# for now, decode as a script, will make into function soon

if __name__ == "__main__":
    # ------------------------------------------------------------------------------------------
    # file bootfile kind
    file_to_decode_start = Path("./../data/2023-03-09-19-35.dat")

    (kind, data) = decode_file(file_to_decode_start)
    ic(kind)
    ic(data)

    # ------------------------------------------------------------------------------------------
    # file datafile kind
    file_to_decode_data = Path("./../data/2023-03-09-19-42.dat")

    (kind, data) = decode_file(file_to_decode_data)
    ic(kind)
    ic(data)


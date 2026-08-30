#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

#!/usr/bin/env python3
import argparse
import hashlib
import os
import platform
import shutil
import sys

__version__ = "1.0"

SUPPORTED_PLATFORMS = ("Linux", "Darwin", "Windows")


def check_platform():
    current = platform.system()
    if current not in SUPPORTED_PLATFORMS:
        print(f"Error: unsupported platform '{current}'. Supported: Linux, macOS, Windows.", file=sys.stderr)
        sys.exit(1)


def calc_md5(filepath):
    h = hashlib.md5()
    with open(filepath, "rb") as f:
        for chunk in iter(lambda: f.read(8192), b""):
            h.update(chunk)
    return h.hexdigest()


def rename_with_md5(filepath):
    # Resolve to absolute path; on Windows this also normalises drive-letter casing
    filepath = os.path.abspath(filepath)
    if not os.path.isfile(filepath):
        print(f"Error: '{filepath}' is not a file or does not exist.", file=sys.stderr)
        sys.exit(1)

    md5 = calc_md5(filepath)
    dirpath = os.path.dirname(filepath)
    basename = os.path.basename(filepath)
    name, ext = os.path.splitext(basename)

    new_name = f"{name}-{md5}{ext}"
    new_path = os.path.join(dirpath, new_name)

    shutil.copy2(filepath, new_path)  # copy with metadata, keep source intact
    print(new_name)


def main():
    parser = argparse.ArgumentParser(
        description="Rename a file by appending its MD5 hash to the filename."
    )
    parser.add_argument("-f", required=True, metavar="FILE", help="File to rename")
    parser.add_argument("--version", action="version", version=f"%(prog)s {__version__}")
    args = parser.parse_args()

    check_platform()
    rename_with_md5(args.f)


if __name__ == "__main__":
    print(calc_md5(sys.argv[1]))
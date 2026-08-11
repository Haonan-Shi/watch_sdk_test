#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

#!/usr/bin/env python3
"""
Generate flash_image_bank0.json and flash_image_bank1.json for mpcli flash programming.

Flash addresses are read from flash_map.h or flash_map.ini under
  <bin-dir>/flashmap/<flash-size>/
Image files are discovered by scanning <bin-dir>.

Usage:
  python gen_flash_image_json.py -d <bin-dir> [--flash_map flashmap/4M]
                                 [--app <appname>] [--output <prefix>]
"""

import argparse
import json
import os
import re
import sys
from pathlib import Path

__version__ = "1.0"

# File name prefixes that identify system/platform images (not the app binary).
SYSTEM_PREFIXES = (
    "boot_patch0_", "boot_patch1_", "upperstack_",
    "OTAHeader_", "stack_patch_", "sys_patch_",
    "dsp_sys_image_", "dsp_app_image_", "dsp_config_image",
    "SYSTEM_Config_", "APP_Config_", "VPData_", "root_",
)

# Each slot: (flash_map macro, glob pattern, bank_hint)
# bank_hint "BANK0" / "BANK1" → prefer path components with that name
# bank_hint "rcfg"            → search under rcfg/
# pattern None                → app binary (auto-detected separately)
BANK0_SLOTS = [
    ("OEM_CFG_ADDR",           "SYSTEM_Config_*.bin",         "rcfg"),
    ("BOOT_PATCH0_ADDR",       "boot_patch0_*.bin",           "BANK0"),
    ("BOOT_PATCH1_ADDR",       "boot_patch1_*.bin",           "BANK0"),
    ("UPPERSTACK_ADDR",        "upperstack_*.bin",            "BANK0"),
    ("BANK0_OTA_HDR_ADDR",     "OTAHeader_Bank0_*.bin",       "BANK0"),
    ("BANK0_STACK_PATCH_ADDR", "stack_patch_bank0_*.bin",     "BANK0"),
    ("BANK0_SYS_PATCH_ADDR",   "sys_patch_bank0_*.bin",       "BANK0"),
    ("BANK0_APP_ADDR",         None,                          "BANK0"),
    ("BANK0_DSP_SYS_ADDR",     "dsp_sys_image_*.bin",         "BANK0"),
    ("BANK0_DSP_APP_ADDR",     "dsp_app_image_*.bin",         "BANK0"),
    ("BANK0_DSP_CFG_ADDR",     "dsp_config_image*.bin",      "BANK0"),
    ("BANK0_APP_CFG_ADDR",     "APP_Config_*.bin",            "rcfg"),
    ("VP_DATA_ADDR",           "VPData_*.bin",                "rcfg"),
    ("USER_DATA1_ADDR",        "root_*.bin",                  None),
]

BANK1_SLOTS = [
    ("OEM_CFG_ADDR",           "SYSTEM_Config_*.bin",         "rcfg"),
    ("BOOT_PATCH0_ADDR",       "boot_patch0_*.bin",           "BANK1"),
    ("BOOT_PATCH1_ADDR",       "boot_patch1_*.bin",           "BANK1"),
    ("UPPERSTACK_ADDR",        "upperstack_*.bin",            "BANK1"),
    ("BANK1_OTA_HDR_ADDR",     "OTAHeader_Bank1_*.bin",       "BANK1"),
    ("BANK1_STACK_PATCH_ADDR", "stack_patch_bank1_*.bin",     "BANK1"),
    ("BANK1_SYS_PATCH_ADDR",   "sys_patch_bank1_*.bin",       "BANK1"),
    ("BANK1_APP_ADDR",         None,                          "BANK1"),
    ("BANK1_DSP_SYS_ADDR",     "dsp_sys_image_*.bin",         "BANK1"),
    ("BANK1_DSP_APP_ADDR",     "dsp_app_image_*.bin",         "BANK1"),
    ("BANK1_DSP_CFG_ADDR",     "dsp_config_image*.bin",      "BANK1"),
    ("BANK1_APP_CFG_ADDR",     "APP_Config_*.bin",            "rcfg"),
    ("VP_DATA_ADDR",           "VPData_*.bin",                "rcfg"),
    ("USER_DATA1_ADDR",        "root_*.bin",                  None),
]


# ---------------------------------------------------------------------------
# Flash map parsing
# ---------------------------------------------------------------------------

def _parse_flash_map_h(path):
    addrs = {}
    pattern = re.compile(r'#define\s+(\w+_ADDR)\s+(0x[0-9A-Fa-f]+)')
    with open(path) as f:
        for line in f:
            m = pattern.search(line)
            if m:
                addrs[m.group(1)] = int(m.group(2), 16)
    return addrs


def _parse_flash_map_ini(path):
    addrs = {}
    pattern = re.compile(r'^(\w+_ADDR)\s*=\s*(0x[0-9A-Fa-f]+)', re.IGNORECASE)
    with open(path) as f:
        for line in f:
            m = pattern.match(line.strip())
            if m:
                addrs[m.group(1)] = int(m.group(2), 16)
    return addrs


def load_flash_map(flashmap_dir):
    h = os.path.join(flashmap_dir, "flash_map.h")
    ini = os.path.join(flashmap_dir, "flash_map.ini")
    if os.path.isfile(h):
        print(f"  Using: {h}")
        return _parse_flash_map_h(h)
    if os.path.isfile(ini):
        print(f"  Using: {ini}")
        return _parse_flash_map_ini(ini)
    print(f"Error: no flash_map.h or flash_map.ini in '{flashmap_dir}'", file=sys.stderr)
    sys.exit(1)


def detect_flash_size(bin_dir):
    root = os.path.join(bin_dir, "flashmap")
    if not os.path.isdir(root):
        return None, None
    sizes = sorted(d for d in os.listdir(root) if os.path.isdir(os.path.join(root, d)))
    if not sizes:
        return None, None
    return sizes[0], os.path.join(root, sizes[0])


# ---------------------------------------------------------------------------
# File search
# ---------------------------------------------------------------------------

def _score(path_obj, bank_hint, flash_size):
    parts = path_obj.parts
    s = 0
    if bank_hint in ("BANK0", "BANK1"):
        if bank_hint in parts:
            s += 10
        opp = "BANK1" if bank_hint == "BANK0" else "BANK0"
        if opp in parts:
            s -= 10
    elif bank_hint == "rcfg":
        if "rcfg" in parts:
            s += 10
    if flash_size and flash_size in parts:
        s += 5
    return s


def find_file(bin_dir, pattern, bank_hint, flash_size):
    """Return the best-matching Path for pattern, or None."""
    matches = list(Path(bin_dir).rglob(pattern))
    if not matches:
        return None
    matches.sort(key=lambda p: _score(p, bank_hint, flash_size), reverse=True)
    best = matches[0]
    if len(matches) > 1 and _score(best, bank_hint, flash_size) == _score(matches[1], bank_hint, flash_size):
        print(f"  WARNING: ambiguous match for '{pattern}': using {best.name}", file=sys.stderr)
    return best


def find_app_binary(bin_dir, bank_hint, flash_size, app_path=None):
    """
    Locate the app binary for the given bank.

    If app_path is given, treat it as the parent directory of BANK0/BANK1
    (relative to bin_dir or absolute), e.g.
      app_demo_bin/bt_audio_trx/4M/8763GTS/bt_audio_transceiver/device2
    For BANK1, fall back to BANK0 if no BANK1 subdirectory exists.
    If app_path is not given, auto-detect by scanning for non-system .bin
    files in BANK0/BANK1 dirs under bin_dir.
    """
    if app_path:
        p = Path(app_path)
        if not p.is_absolute():
            p = Path(bin_dir) / p
        bank_dir = p / bank_hint
        if not bank_dir.is_dir():
            if bank_hint == "BANK1":
                bank_dir = p / "BANK0"
                if bank_dir.is_dir():
                    print(f"  No BANK1 dir, falling back to BANK0 binary",
                          file=sys.stderr)
            if not bank_dir.is_dir():
                print(f"  WARNING: app directory not found: {bank_dir}", file=sys.stderr)
                return None
        matches = list(bank_dir.glob("*.bin"))
        if not matches:
            print(f"  WARNING: no .bin found in {bank_dir}", file=sys.stderr)
            return None
        if len(matches) > 1:
            print(f"  WARNING: multiple .bin in {bank_dir}, using: {matches[0].name}",
                  file=sys.stderr)
        return matches[0]

    # Auto-detection fallback
    candidates = []
    for f in Path(bin_dir).rglob("*.bin"):
        if bank_hint not in f.parts:
            continue
        if any(f.name.startswith(pfx) for pfx in SYSTEM_PREFIXES):
            continue
        if flash_size and flash_size not in f.parts:
            continue
        candidates.append(f)

    if not candidates:
        return None
    if len(candidates) > 1:
        print(f"  WARNING: multiple app binaries found for {bank_hint}:", file=sys.stderr)
        for c in candidates:
            print(f"    {c.relative_to(bin_dir)}", file=sys.stderr)
        print(f"  Using: {candidates[0].name}  (use --app to specify)", file=sys.stderr)
    return candidates[0]


# ---------------------------------------------------------------------------
# JSON generation
# ---------------------------------------------------------------------------

def build_json(bin_dir, json_out, slots, flash_addrs, flash_size, app_binary):
    json_dir = Path(json_out).parent
    files = []

    for macro, pattern, bank_hint in slots:
        addr = flash_addrs.get(macro)
        if addr is None or addr == 0:
            continue

        if pattern is None:
            fp = app_binary
        else:
            fp = find_file(bin_dir, pattern, bank_hint, flash_size)

        if fp is None:
            print(f"  WARNING: file not found for {macro} ({pattern}), slot skipped",
                  file=sys.stderr)
            continue

        rel = fp.relative_to(json_dir)
        rel_str = "./" + str(rel).replace(os.sep, "/")
        files.append({"address": hex(addr), "path": rel_str})

    return {"mptoolconfig": {"port": "COM1", "baud": "1000000",
                             "images": {"file": files}}}


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="Generate flash_image_bank0.json / flash_image_bank1.json for mpcli."
    )
    parser.add_argument("-d", "--bin-dir", required=True, metavar="DIR",
                        help="Image bin directory (e.g. .../bin/rtl87x3g)")
    parser.add_argument("--flash_map", metavar="DIR",
                        help="Path to flash map directory (relative to bin-dir or absolute,"
                             " e.g. flashmap/4M); auto-detected if omitted")
    parser.add_argument("--app", metavar="DIR",
                        help="Parent directory of BANK0/BANK1, relative to bin-dir or absolute"
                             " (e.g. app_demo_bin/bt_audio_trx/4M/8763GTS/bt_audio_transceiver/device2);"
                             " auto-detected if omitted")
    parser.add_argument("--output", metavar="PREFIX", default="flash_image",
                        help="Output JSON filename prefix (default: flash_image);"
                             " generates <PREFIX>_bank0.json and <PREFIX>_bank1.json")
    parser.add_argument("--version", action="version", version=f"%(prog)s {__version__}")
    args = parser.parse_args()

    bin_dir = os.path.abspath(args.bin_dir)
    if not os.path.isdir(bin_dir):
        print(f"Error: '{bin_dir}' is not a directory.", file=sys.stderr)
        sys.exit(1)

    # Resolve flash map directory and flash size label
    if args.flash_map:
        p = Path(args.flash_map)
        flashmap_dir = str(p if p.is_absolute() else Path(bin_dir) / p)
        flash_size = os.path.basename(flashmap_dir)
    else:
        flash_size, flashmap_dir = detect_flash_size(bin_dir)
        if flashmap_dir is None:
            print("Error: cannot detect flash map directory; use --flash_map.", file=sys.stderr)
            sys.exit(1)
        print(f"Detected flash map: {flashmap_dir}")

    if not os.path.isdir(flashmap_dir):
        print(f"Error: flash map directory not found: '{flashmap_dir}'", file=sys.stderr)
        sys.exit(1)

    print(f"\nLoading flash map ({flash_size})...")
    flash_addrs = load_flash_map(flashmap_dir)
    print(f"  {len(flash_addrs)} address entries loaded")

    # Locate app binaries
    print("\nSearching for Bank0 app binary...")
    app_bank0 = find_app_binary(bin_dir, "BANK0", flash_size, args.app)
    if app_bank0:
        print(f"  {Path(app_bank0).relative_to(bin_dir)}")
    else:
        print("  Not found — BANK0_APP_ADDR slot will be skipped")

    print("\nSearching for Bank1 app binary...")
    app_bank1 = find_app_binary(bin_dir, "BANK1", flash_size, args.app)
    if app_bank1:
        print(f"  {Path(app_bank1).relative_to(bin_dir)}")
    else:
        print("  Not found — BANK1_APP_ADDR slot will be skipped")

    # Generate Bank0 JSON
    out0 = os.path.join(bin_dir, f"{args.output}_bank0.json")
    print(f"\nGenerating {os.path.basename(out0)}...")
    data0 = build_json(bin_dir, out0, BANK0_SLOTS, flash_addrs, flash_size, app_bank0)
    with open(out0, "w") as f:
        json.dump(data0, f, indent=2)
    n0 = len(data0["mptoolconfig"]["images"]["file"])
    print(f"  {out0}  ({n0} images)")

    # Generate Bank1 JSON
    out1 = os.path.join(bin_dir, f"{args.output}_bank1.json")
    print(f"\nGenerating {os.path.basename(out1)}...")
    data1 = build_json(bin_dir, out1, BANK1_SLOTS, flash_addrs, flash_size, app_bank1)
    with open(out1, "w") as f:
        json.dump(data1, f, indent=2)
    n1 = len(data1["mptoolconfig"]["images"]["file"])
    print(f"  {out1}  ({n1} images)")


if __name__ == "__main__":
    main()

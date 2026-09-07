#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
flash_autoreset.py — RTS reset helper (pure RTS control, does not run mpcli)

Responsibility boundary:
  This script ONLY uses the RTS line to drop the RTL87X3G board into UART
  download mode; it never touches mpcli. mpcli is started by the mpcli west
  runner, which spawns this script concurrently when it sees --auto.
  => mpcli is launched from exactly one place (the runner); the two scripts
  have a clean separation of duties.

Timing: mpcli starts sending handshake packets ~0.8s
  after launch; a single RTS reset pulse drops the board into the MP loader,
  and the handshake succeeds ~0.5s later; erase does not begin until ~2s. So
  this script waits --delay s -> pulses --count times -> exits, all before
  erase, so the reset pulse can never interrupt flashing. The board does not
  wait in the loader, so the pulse must land while mpcli is still sending
  handshake packets (hence it runs concurrently with mpcli).

Wiring on this board (single line): RTS -> two-stage NPN -> nRESET (net
  non-inverting); P2_0 is held low in hardware, so releasing reset enters the
  loader.
  assert   RTS (rts=True)  -> nRESET low  = reset asserted
  deassert RTS (rts=False) -> nRESET high = released, enters loader
Note: the serial port driving reset may not be the flash port (different
  adapter); use --port to select the reset port.

Usage (normally invoked as a subprocess by the runner, but also manually):
  ./flash_autoreset.py --port /dev/ttyUSB0
"""
import argparse
import signal
import sys
import time

CTL_BAUD = 1000000       # control-handle baud rate (measured to be insensitive)
RESET_DELAY = 1.5        # wait before first reset (let mpcli come up and start
                         # the handshake, with margin for slow startup)
RESET_HOLD = 0.12        # how long to hold reset low
RESET_COUNT = 1          # number of reset pulses (one connects, well before erase)
RESET_INTERVAL = 0.4     # interval between pulses (only used when count > 1)


def main():
    ap = argparse.ArgumentParser(
        description="RTS reset helper (RTS only, does not run mpcli)")
    ap.add_argument("-c", "--port", required=True,
                    help="serial port driving reset via RTS (may differ from flash port)")
    ap.add_argument("--baud", type=int, default=CTL_BAUD,
                    help="control-handle baud rate (measured to be insensitive)")
    ap.add_argument("--delay", type=float, default=RESET_DELAY,
                    help="seconds to wait before first reset (until mpcli starts handshake)")
    ap.add_argument("--hold", type=float, default=RESET_HOLD,
                    help="seconds to hold reset low")
    ap.add_argument("--count", type=int, default=RESET_COUNT,
                    help="number of reset pulses")
    ap.add_argument("--interval", type=float, default=RESET_INTERVAL,
                    help="seconds between pulses")
    args = ap.parse_args()

    try:
        import serial
    except ImportError:
        raise SystemExit("flash_autoreset.py requires pyserial: pip install pyserial")

    # rtscts/dsrdtr=False: don't let pyserial drive the flow-control lines
    # automatically; control RTS by hand only.
    ser = serial.Serial(args.port, args.baud, rtscts=False, dsrdtr=False, timeout=0.2)
    ser.rts = False   # released at start (reset not asserted)

    def release_and_exit(*_):
        # If the runner sends SIGTERM early: release RTS before exiting so the
        # board isn't left held in reset.
        try:
            ser.rts = False
        except Exception:
            pass
        try:
            ser.close()
        except Exception:
            pass
        sys.exit(0)

    signal.signal(signal.SIGTERM, release_and_exit)
    signal.signal(signal.SIGINT, release_and_exit)

    print(f"[autoreset] RTS reset: port {args.port} | pulse {args.count}x after {args.delay}s",
          flush=True)
    time.sleep(args.delay)
    for i in range(args.count):
        print(f"[autoreset] reset pulse {i + 1}/{args.count}", flush=True)
        ser.rts = True       # assert reset (nRESET low)
        time.sleep(args.hold)
        ser.rts = False      # release reset -> enter loader
        if i < args.count - 1:
            time.sleep(args.interval)
    ser.rts = False
    ser.close()


if __name__ == "__main__":
    main()

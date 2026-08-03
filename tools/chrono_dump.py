#!/usr/bin/env python3
"""
chrono_dump.py - Unified ChronoLog log fetch/parse tool.

Handles BOTH inputs with a single shared parse/compact core:
  --port <port>          MCU fetch mode: send 'F' over UART, read the framed log
                         stream, decode, write logs/*.
  --dir <path>           Desktop mode: read local ChronoLog log files directly.
  (no location)          Prompts the user for a location, defaulting to logs/*.

Detects the ChronoLog signature ("# chrono-log sig") and reconstructs
console-equivalent lines. Pure stdlib (pyserial optional).

Usage:
  python3 tools/chrono_dump.py --port /dev/ttyUSB0 --baud 460800 --out logs/
  python3 tools/chrono_dump.py --dir logs/ --out logs_compact/
  python3 tools/chrono_dump.py            # prompts for location, defaults to logs/*
"""

import argparse
import os
import sys
import re

CHRONO_SIGNATURE = "# chrono-log sig"

# Abbreviated-level -> full level (shared with the C++ sinks).
LEVEL_SHORT_TO_FULL = {
    "I": "INFO",
    "D": "DEBUG",
    "W": "WARN",
    "E": "ERROR",
    "F": "FATAL",
    "P": "PROGRESS",
}


def is_chrono_log_file(path):
    """Detect the ChronoLog signature in a log file."""
    try:
        with open(path, "r", encoding="utf-8", errors="replace") as f:
            head = f.read(512)
        return CHRONO_SIGNATURE in head
    except Exception:
        return False


def expand_line(line, module):
    """Expand a (possibly condensed) ChronoLog line to full console form.

    Handles:
      full-fidelity : "HH:MM:SS | module | LEVEL | task | message"
      condensed     : "HH:MM:SS |L| message"   (module from the file header)
    """
    line = line.rstrip("\r\n")
    if not line or line.startswith("#") or line.startswith("module =") or line.startswith("created ="):
        return None

    # Condensed: HH:MM:SS |L| T<n> | message
    m = re.match(r"^(\d\d:\d\d:\d\d) \|([IDWEFP])\| T\d+ \| (.*)$", line)
    if m:
        return "{} | {:<15} | {:<8} | {:<16} | {}".format(
            m.group(1), module, LEVEL_SHORT_TO_FULL.get(m.group(2), m.group(2)), "", m.group(3))

    # Full-fidelity: HH:MM:SS | module | LEVEL | task | message
    m = re.match(r"^(\d\d:\d\d:\d\d) \| ([^|]+) \| ([^|]+) \| ([^|]+) \| (.*)$", line)
    if m:
        return "{} | {} | {} | {} | {}".format(
            m.group(1), m.group(2).strip(), m.group(3).strip(),
            m.group(4).strip(), m.group(5))

    return None


def parse_file(path, out_dir):
    """Parse a single ChronoLog file and write the expanded version."""
    if not is_chrono_log_file(path):
        print("  skip (not a ChronoLog file): {}".format(path))
        return 0

    module = None
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        for line in f:
            if line.startswith("module ="):
                m = re.search(r'"([^"]+)"', line)
                if m:
                    module = m.group(1)
                break

    if not module:
        module = os.path.splitext(os.path.basename(path))[0]

    out_path = os.path.join(out_dir, "{}.txt".format(module))
    count = 0
    with open(path, "r", encoding="utf-8", errors="replace") as fin, \
         open(out_path, "a", encoding="utf-8") as fout:
        fout.write("# chrono-log expanded (from {})\n".format(os.path.basename(path)))
        for line in fin:
            expanded = expand_line(line, module)
            if expanded:
                fout.write(expanded + "\n")
                count += 1
    return count


def process_dir(in_dir, out_dir):
    """Desktop mode: parse all ChronoLog files in a directory."""
    if not os.path.isdir(in_dir):
        print("error: not a directory: {}".format(in_dir))
        return 1
    os.makedirs(out_dir, exist_ok=True)
    total = 0
    for fname in sorted(os.listdir(in_dir)):
        path = os.path.join(in_dir, fname)
        if os.path.isfile(path):
            total += parse_file(path, out_dir)
    print("Parsed {} lines into {}".format(total, out_dir))
    return 0


def process_port(port, baud, out_dir):
    """MCU fetch mode: send 'F', read framed stream, decode, write logs."""
    try:
        import serial
    except ImportError:
        print("error: pyserial not installed. `pip install pyserial`, or use --dir for desktop mode.")
        return 1

    os.makedirs(out_dir, exist_ok=True)
    print("Fetching logs from {} @ {} baud...".format(port, baud))
    ser = serial.Serial(port, baud, timeout=2)
    ser.reset_input_buffer()
    ser.write(b"F")  # fetch command

    data = b""
    while True:
        chunk = ser.read(256)
        if not chunk:
            break
        data += chunk

    ser.close()

    # Decode the framed stream: files are delimited; here we parse as text lines.
    text = data.decode("utf-8", errors="replace")
    # Split into per-module files by the signature header.
    files = re.split(r"(?=# chrono-log sig)", text)
    total = 0
    for block in files:
        if not block.strip():
            continue
        module = "mcu"
        m = re.search(r'module = "([^"]+)"', block)
        if m:
            module = m.group(1)
        out_path = os.path.join(out_dir, "{}.txt".format(module))
        with open(out_path, "w", encoding="utf-8") as f:
            f.write(block)
            total += block.count("\n")
    print("Fetched {} bytes -> {} lines -> {}".format(len(data), total, out_dir))
    return 0


def prompt_for_location():
    print("No location given.")
    choice = input("Fetch from MCU port (e.g. /dev/ttyUSB0) or read a directory (e.g. logs)? [dir] ").strip()
    if not choice:
        return None, "logs"
    if os.path.isdir(choice):
        return "dir", choice
    return "port", choice


def main():
    parser = argparse.ArgumentParser(description="ChronoLog log fetch/parse tool")
    parser.add_argument("--port", help="MCU UART port (MCU fetch mode)")
    parser.add_argument("--baud", type=int, default=460800, help="UART baud rate (default 460800)")
    parser.add_argument("--dir", help="Local log directory (desktop mode)")
    parser.add_argument("--out", default="logs", help="Output directory (default 'logs')")
    args = parser.parse_args()

    if args.dir:
        return process_dir(args.dir, args.out)
    if args.port:
        return process_port(args.port, args.baud, args.out)

    mode, loc = prompt_for_location()
    if mode == "dir":
        return process_dir(loc, args.out)
    return process_port(loc, args.baud, args.out)


if __name__ == "__main__":
    sys.exit(main())

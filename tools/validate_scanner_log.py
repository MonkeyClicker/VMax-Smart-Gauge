#!/usr/bin/env python3
"""Validate scanner-diag-2 records; report damage and sequence gaps without repair."""
import argparse
import re
from pathlib import Path


def checksum(payload):
    value = 2166136261
    for byte in payload:
        value = ((value ^ byte) * 16777619) & 0xFFFFFFFF
    return value


def validate(data):
    valid = damaged = gaps = 0
    previous = None
    for line in data.splitlines():
        match = re.fullmatch(rb'@(\d+) (.*) \*([0-9A-F]{8})', line)
        if not match:
            if line:
                damaged += 1
            continue
        sequence, payload, expected = match.groups()
        if checksum(payload) != int(expected, 16):
            damaged += 1
            continue
        sequence = int(sequence)
        if previous is not None:
            # Sequence restarts at boot; repeated or backward records are flagged.
            if sequence <= previous:
                damaged += 1
            else:
                gaps += sequence - previous - 1
        previous = sequence
        valid += 1
    return valid, damaged, gaps


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('capture', type=Path)
    args = parser.parse_args()
    valid, damaged, gaps = validate(args.capture.read_bytes())
    print(f'valid={valid} damaged_or_unframed={damaged} missing_records={gaps}')
    raise SystemExit(0 if valid and not damaged and not gaps else 1)

#!/usr/bin/env python3
"""Verify the installed kernel using upstream's production instruction contracts."""
import argparse
from pathlib import Path
import subprocess
import sys

root = Path(__file__).resolve().parents[1] / 'source/WhateverXe'
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--kernel', type=Path, default=Path('/System/Library/Kernels/kernel'))
parser.add_argument('--hde-source', type=Path, required=True,
                    help='hde/hde64.c from the Lilu 1.7.2 source checkout')
args = parser.parse_args()
subprocess.run([sys.executable, str(root / 'tests/sequoia_pmap.py'),
                '--kernel', str(args.kernel), '--hde-source', str(args.hde_source)], check=True)

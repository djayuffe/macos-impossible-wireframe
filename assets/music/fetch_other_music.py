#!/usr/bin/env python3
"""Optional helper stub for a non-previous demo music music module.

This project deliberately does not re-host tracker music. Put a legally usable
MOD/S3M/XM in this directory and run the demo with a matching `--bpm`.
"""
from pathlib import Path

out = Path(__file__).resolve().parent
print(f"Place a non-previous demo music tracker module in: {out}")
print("Example run: ./build/impossible_wireframe --bpm 132")

#!/usr/bin/env python3
"""Tracked-only repository status helper for bobfilez.

Why this exists:
- Some Windows environments emit noisy filename-too-long warnings during
  full untracked-file scans (`git status`) due deeply nested generated test
  artifacts (notably pybind11/CMake test trees).
- In practice, `git status --untracked-files=no` provides a stable signal for
  tracked changes and dirty submodules without triggering that warning path.

This helper standardizes that safer status view for day-to-day repo hygiene.
"""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path


def run(*args: str) -> int:
    proc = subprocess.run(args, cwd=Path(__file__).resolve().parents[1])
    return proc.returncode


def main() -> int:
    print("== bobfilez tracked-only status ==")
    rc = run("git", "status", "--short", "--branch", "--untracked-files=no")
    print()
    print("== note ==")
    print(
        "Use full `git status` only when you intentionally need untracked-file "
        "visibility; on this repo/host it may emit long-path warnings from generated "
        "test artifacts."
    )
    return rc


if __name__ == "__main__":
    sys.exit(main())

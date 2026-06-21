#!/usr/bin/env python3
"""Validate BDD block structure in changed C test functions."""

from __future__ import annotations

import argparse
import os
import re
import subprocess
import sys
from pathlib import Path


BDD_RE = re.compile(r"^\s*(?://|/\*)\s*(Given|When|Then|Cleanup)\b")
TEST_RE = re.compile(r"^static\s+void\s+(test_[A-Za-z0-9_]+)\s*\(\s*void\s*\)\s*\{")
HUNK_RE = re.compile(r"^@@ -\d+(?:,\d+)? \+(\d+)(?:,(\d+))? @@")


def _run_git(args: list[str]) -> str:
    result = subprocess.run(
        ["git", *args],
        check=False,
        stdout=subprocess.PIPE,
        stderr=subprocess.DEVNULL,
        text=True,
    )
    if result.returncode != 0:
        return ""
    return result.stdout


def _repo_paths(paths: list[str]) -> list[Path]:
    return [
        Path(line)
        for line in paths
        if line.endswith("_tests.c") and Path(line).is_file()
    ]


def _changed_files_from_git() -> list[Path]:
    names: set[str] = set()

    for args in (
        ["diff", "--name-only", "--diff-filter=ACMR"],
        ["diff", "--cached", "--name-only", "--diff-filter=ACMR"],
        ["ls-files", "--others", "--exclude-standard"],
    ):
        names.update(_run_git(args).splitlines())

    base = os.environ.get("AGERUN_TEST_STRUCTURE_BASE")
    if not base and os.environ.get("GITHUB_BASE_REF"):
        base = f"origin/{os.environ['GITHUB_BASE_REF']}"
    if base:
        names.update(
            _run_git(
                ["diff", "--name-only", "--diff-filter=ACMR", f"{base}...HEAD"]
            ).splitlines()
        )

    if not names and _run_git(["rev-parse", "--verify", "HEAD^"]):
        names.update(
            _run_git(
                ["diff", "--name-only", "--diff-filter=ACMR", "HEAD^", "HEAD"]
            ).splitlines()
        )

    return sorted(set(_repo_paths(sorted(names))))


def _all_test_files() -> list[Path]:
    return sorted(path for path in Path("modules").glob("*_tests.c") if path.is_file())


def _changed_lines_from_diff(args: list[str], path: Path) -> set[int]:
    output = _run_git([*args, "--", str(path)])
    changed: set[int] = set()
    for line in output.splitlines():
        match = HUNK_RE.match(line)
        if not match:
            continue
        start = int(match.group(1))
        count = int(match.group(2) or "1")
        changed.update(range(start, start + count))
    return changed


def _changed_lines(path: Path, check_all: bool, explicit: bool) -> set[int] | None:
    lines = path.read_text().splitlines()
    if check_all or explicit:
        return None

    if path.as_posix() in _run_git(
        ["ls-files", "--others", "--exclude-standard"]
    ).splitlines():
        return set(range(1, len(lines) + 1))

    changed: set[int] = set()
    changed.update(_changed_lines_from_diff(["diff", "--unified=0"], path))
    changed.update(_changed_lines_from_diff(["diff", "--cached", "--unified=0"], path))

    base = os.environ.get("AGERUN_TEST_STRUCTURE_BASE")
    if not base and os.environ.get("GITHUB_BASE_REF"):
        base = f"origin/{os.environ['GITHUB_BASE_REF']}"
    if base:
        changed.update(
            _changed_lines_from_diff(["diff", "--unified=0", f"{base}...HEAD"], path)
        )

    if not changed and _run_git(["rev-parse", "--verify", "HEAD^"]):
        changed.update(_changed_lines_from_diff(["diff", "--unified=0", "HEAD^", "HEAD"], path))

    return changed


def _test_ranges(path: Path) -> list[tuple[str, int, int, list[str]]]:
    lines = path.read_text().splitlines()
    ranges: list[tuple[str, int, int, list[str]]] = []
    line_index = 0
    while line_index < len(lines):
        match = TEST_RE.match(lines[line_index])
        if not match:
            line_index += 1
            continue

        name = match.group(1)
        start = line_index + 1
        brace_depth = lines[line_index].count("{") - lines[line_index].count("}")
        end_index = line_index + 1
        while end_index < len(lines) and brace_depth > 0:
            brace_depth += lines[end_index].count("{") - lines[end_index].count("}")
            end_index += 1

        end = end_index
        ranges.append((name, start, end, lines[start:end - 1]))
        line_index = end_index

    return ranges


def _block_starts(
    function_lines: list[str],
    function_start: int,
) -> list[tuple[int, list[str]]]:
    starts: list[tuple[int, list[str]]] = []
    block: list[str] = []
    block_start = function_start + 1

    for offset, line in enumerate(function_lines, start=function_start + 1):
        if line.strip():
            if not block:
                block_start = offset
            block.append(line)
            continue
        if block:
            starts.append((block_start, block))
            block = []

    if block:
        starts.append((block_start, block))

    return starts


def _is_banner_block(block: list[str]) -> bool:
    significant = [line.strip() for line in block if line.strip()]
    return len(significant) == 1 and significant[0].startswith("printf(")


def _validate_function(path: Path, name: str, start: int, lines: list[str]) -> list[str]:
    issues: list[str] = []
    for block_start, block in _block_starts(lines, start):
        first = block[0]
        if _is_banner_block(block) or BDD_RE.match(first):
            continue
        issues.append(
            f"{path}:{block_start}: {name} block must start with "
            "// Given, // When, // Then, or // Cleanup"
        )
    return issues


def _validate_files(paths: list[Path], check_all: bool, explicit: bool) -> list[str]:
    issues: list[str] = []
    checked = 0

    for path in paths:
        changed = _changed_lines(path, check_all, explicit)
        for name, start, end, lines in _test_ranges(path):
            if changed is not None and not any(
                start <= line <= end for line in changed
            ):
                continue
            checked += 1
            issues.extend(_validate_function(path, name, start, lines))

    if checked == 0:
        print("No changed C test functions to check.")
    else:
        print(f"Checked {checked} C test function(s).")
    return issues


def main() -> int:
    parser = argparse.ArgumentParser(
        description=(
            "Require each blank-separated block in C test functions to start "
            "with BDD comments."
        )
    )
    parser.add_argument("files", nargs="*", help="Specific *_tests.c files to check")
    parser.add_argument("--all", action="store_true", help="Check every C test function")
    args = parser.parse_args()

    explicit = bool(args.files)
    if args.all:
        files = _all_test_files()
    elif explicit:
        files = _repo_paths(args.files)
    else:
        files = _changed_files_from_git()

    if not files:
        print("No C test files to check.")
        return 0

    issues = _validate_files(files, args.all, explicit)
    if issues:
        print("BDD test structure violations:")
        for issue in issues:
            print(f"  - {issue}")
        return 1

    print("BDD test structure check passed.")
    return 0


if __name__ == "__main__":
    sys.exit(main())

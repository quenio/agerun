#!/usr/bin/env python3
"""
DEPRECATED: This script compacts CHANGELOG to single-line format (hard to read).

As of 2025-10-18, CHANGELOG.md uses multi-line format for better human readability.
This script is preserved for reference but should NOT be used for CHANGELOG.md.

For CHANGELOG compaction, use MANUAL editing following these principles:
1. MAINTAIN multi-line format (do NOT compress to single lines)
2. Reduce verbosity WITHIN sections, not by combining sections
3. See: kb/documentation-compacting-pattern.md for guidance
4. See: .opencode/command/ar/compact-changes.md for workflow

This script may still be useful for OTHER documents that use single-line format.
"""

from __future__ import annotations

import argparse
import pathlib
import re
import sys
from typing import List


BULLET_PATTERN = re.compile(r"^\s*-\s+")


SEGMENTS_PER_LINE = 2


def _format_bullet_block(block_lines: List[str]) -> str:
    """Convert a block of bullet lines into compact bullet lines."""
    segments: List[str] = []

    for raw_line in block_lines:
        stripped = raw_line.lstrip()
        indent = len(raw_line) - len(stripped)

        if not stripped.startswith("-"):
            text = stripped.strip()
            if text:
                if segments:
                    segments[-1] = f"{segments[-1]} {text}"
                else:
                    segments.append(text)
            continue

        content = stripped[1:].lstrip()

        if indent == 0:
            segments.append(content)
            continue

        if not segments:
            segments.append(content)
            continue

        if segments[-1].endswith(":"):
            segments[-1] = f"{segments[-1]} {content}"
        else:
            segments[-1] = f"{segments[-1]}; {content}"

    compact_lines = []
    for start in range(0, len(segments), SEGMENTS_PER_LINE):
        chunk = [segment.rstrip() for segment in segments[start : start + SEGMENTS_PER_LINE]]
        compact_lines.append(f"- {'; '.join(chunk)}")

    return "\n".join(compact_lines)


def _compact_content(original: str) -> str:
    """Produce compacted changelog content without modifying code blocks."""
    lines = original.splitlines()
    output: List[str] = []
    idx = 0
    in_code_block = False

    while idx < len(lines):
        line = lines[idx]
        stripped = line.strip()

        if stripped.startswith("```"):
            output.append(line)
            in_code_block = not in_code_block
            idx += 1
            continue

        if not in_code_block and BULLET_PATTERN.match(line):
            block: List[str] = []

            while idx < len(lines):
                candidate = lines[idx]
                if BULLET_PATTERN.match(candidate):
                    block.append(candidate)
                    idx += 1
                    continue

                # End of bullet block (blank line ends block as well)
                if candidate.strip() == "":
                    break
                if candidate.strip().startswith("```"):
                    break
                if candidate.startswith(" "):
                    # Continuation line – treat as part of previous bullet
                    block.append(candidate)
                    idx += 1
                    continue
                break

            output.append(_format_bullet_block(block))
            continue

        output.append(line)
        idx += 1

    return "\n".join(output) + "\n"


def _write_output(path: pathlib.Path, content: str) -> None:
    path.write_text(content, encoding="utf-8")


def compact_changelog(changelog_path: pathlib.Path, apply_changes: bool) -> int:
    original = changelog_path.read_text(encoding="utf-8")
    compacted = _compact_content(original)

    original_lines = original.count("\n") + (0 if original.endswith("\n") else 1)
    new_lines = compacted.count("\n")
    reduction = 0 if original_lines == 0 else int(
        (original_lines - new_lines) * 100 / original_lines
    )

    temp_path = changelog_path.with_suffix(".md.new")
    _write_output(temp_path, compacted)

    if not apply_changes:
        print(
            "DRY RUN: Would compact "
            f"{changelog_path.name} from {original_lines} to {new_lines} lines "
            f"({reduction}% reduction)"
        )
        print(f"Preview written to {temp_path}")
        return 0

    if reduction < 40:
        print(
            "❌ FAILURE: Only "
            f"{reduction}% reduction achieved (target: 40-50% for historical records)"
        )
        return 1

    _write_output(changelog_path, compacted)
    print(
        f"Applied compaction: {changelog_path.name} now {new_lines} lines "
        f"({reduction}% reduction)"
    )
    print(f"Backup written to {temp_path}")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Compact CHANGELOG.md while preserving metrics and dates."
    )
    parser.add_argument(
        "--apply",
        action="store_true",
        help="Apply changes to CHANGELOG.md (default: dry run only).",
    )
    parser.add_argument(
        "--path",
        default="CHANGELOG.md",
        help="Path to changelog file (default: CHANGELOG.md).",
    )
    args = parser.parse_args()

    changelog_path = pathlib.Path(args.path)
    if not changelog_path.exists():
        print(f"❌ {changelog_path} not found")
        return 1

    return compact_changelog(changelog_path, args.apply)


if __name__ == "__main__":
    sys.exit(main())

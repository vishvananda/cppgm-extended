#!/usr/bin/env python3
"""Report whether promoted reducers duplicate their original PA23 tests."""

from __future__ import annotations

import argparse
import csv
import re
import sys
from collections import Counter
from difflib import SequenceMatcher
from pathlib import Path


DEFAULT_TRACKER = Path("pa23_feature_backfill_tracker.tsv")
DEFAULT_ACTION_STATUS = "test-added"
DEFAULT_REVIEWED_DISPOSITIONS = Path("analysis/pa23-original-disposition-overrides.tsv")


def normalized_source(text: str) -> str:
    lines: list[str] = []
    for line in text.splitlines():
        stripped = line.strip()
        if not stripped or stripped.startswith("//"):
            continue
        lines.append(re.sub(r"\s+", " ", stripped))
    return "\n".join(lines)


def disposition(byte_exact: bool, normalized_exact: bool, similarity: float) -> str:
    if byte_exact or normalized_exact:
        return "retire-pa23-duplicate"
    if similarity >= 0.98:
        return "review-retire-or-simplify"
    if similarity >= 0.90:
        return "review-near-duplicate"
    return "keep-pa23-integration-candidate"


def reason_for(result: str) -> str:
    if result == "retire-pa23-duplicate":
        return (
            "promoted reducer is source-identical after formatting/comment normalization; "
            "PA23 copy is duplicate unless rewritten into real integration coverage"
        )
    if result == "review-retire-or-simplify":
        return (
            "promoted reducer is almost the same source; inspect before keeping both"
        )
    if result == "review-near-duplicate":
        return (
            "promoted reducer keeps the same broad shape; decide whether PA23 adds "
            "a meaningful composition ingredient"
        )
    return (
        "promoted reducer is materially smaller or different; PA23 source remains "
        "a plausible integration test"
    )


def load_reviewed_dispositions(path: str) -> dict[str, dict[str, str]]:
    review_path = Path(path)
    if not review_path.exists():
        return {}
    with review_path.open(newline="") as f:
        rows = csv.DictReader(f, delimiter="\t")
        return {row["test"]: row for row in rows}


def apply_reviewed_disposition(row: dict[str, str], reviewed: dict[str, dict[str, str]]) -> dict[str, str]:
    review = reviewed.get(row["test"])
    if not review:
        return row
    row = dict(row)
    row["disposition"] = review["final_disposition"]
    row["reason"] = review["reason"]
    return row


def build_rows(args: argparse.Namespace) -> list[dict[str, str]]:
    tracker_path = Path(args.tracker)
    with tracker_path.open(newline="") as f:
        tracker_rows = list(csv.DictReader(f, delimiter="\t"))

    reviewed = load_reviewed_dispositions(args.reviewed_dispositions)
    output: list[dict[str, str]] = []
    for row in tracker_rows:
        if args.action_status and row.get("action_status") != args.action_status:
            continue
        original_path = Path(row["test"])
        reducer_path = Path(row["proposed_test_path"])
        if not original_path.exists() and reducer_path.exists():
            output.append(
                {
                    "disposition": "pa23-original-removed",
                    "similarity": "",
                    "duplicate_kind": "",
                    "test": row["test"],
                    "reducer": row["proposed_test_path"],
                    "owner": row["candidate_owner_pa"],
                    "classification": row["classification"],
                    "feature_cluster": row["feature_cluster"],
                    "reason": "PA23 original has been removed; promoted reducer still exists",
                }
            )
            continue
        if original_path.exists() and not reducer_path.exists():
            output.append(
                {
                    "disposition": "missing-reducer",
                    "similarity": "",
                    "duplicate_kind": "",
                    "test": row["test"],
                    "reducer": row["proposed_test_path"],
                    "owner": row["candidate_owner_pa"],
                    "classification": row["classification"],
                    "feature_cluster": row["feature_cluster"],
                    "reason": "promoted reducer path is missing",
                }
            )
            continue
        if not original_path.exists() and not reducer_path.exists():
            output.append(
                {
                    "disposition": "missing-file",
                    "similarity": "",
                    "duplicate_kind": "",
                    "test": row["test"],
                    "reducer": row["proposed_test_path"],
                    "owner": row["candidate_owner_pa"],
                    "classification": row["classification"],
                    "feature_cluster": row["feature_cluster"],
                    "reason": "both original and promoted reducer paths are missing",
                }
            )
            continue

        original = original_path.read_text(errors="replace")
        reducer = reducer_path.read_text(errors="replace")
        original_norm = normalized_source(original)
        reducer_norm = normalized_source(reducer)
        byte_exact = original == reducer
        normalized_exact = original_norm == reducer_norm
        similarity = SequenceMatcher(a=original_norm, b=reducer_norm).ratio()
        duplicate_kind = (
            "byte"
            if byte_exact
            else "normalized"
            if normalized_exact
            else ""
        )
        result = disposition(byte_exact, normalized_exact, similarity)
        output.append(
            apply_reviewed_disposition(
                {
                    "disposition": result,
                    "similarity": f"{similarity:.3f}",
                    "duplicate_kind": duplicate_kind,
                    "test": row["test"],
                    "reducer": row["proposed_test_path"],
                    "owner": row["candidate_owner_pa"],
                    "classification": row["classification"],
                    "feature_cluster": row["feature_cluster"],
                    "reason": reason_for(result),
                },
                reviewed,
            )
        )
    return output


def emit_summary(rows: list[dict[str, str]]) -> None:
    counts = Counter(row["disposition"] for row in rows)
    for name in (
        "pa23-original-removed",
        "keep-pa23-integration",
        "retire-pa23-duplicate",
        "review-retire-or-simplify",
        "review-near-duplicate",
        "keep-pa23-integration-candidate",
        "missing-reducer",
        "missing-file",
    ):
        if counts[name]:
            print(f"{name}\t{counts[name]}")


def emit_tsv(rows: list[dict[str, str]]) -> None:
    fieldnames = [
        "disposition",
        "similarity",
        "duplicate_kind",
        "test",
        "reducer",
        "owner",
        "classification",
        "feature_cluster",
        "reason",
    ]
    writer = csv.DictWriter(sys.stdout, fieldnames=fieldnames, delimiter="\t")
    writer.writeheader()
    writer.writerows(rows)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--tracker",
        default=str(DEFAULT_TRACKER),
        help=f"tracker TSV to read (default: {DEFAULT_TRACKER})",
    )
    parser.add_argument(
        "--action-status",
        default=DEFAULT_ACTION_STATUS,
        help=(
            "tracker action_status to report; pass an empty string for all rows "
            f"(default: {DEFAULT_ACTION_STATUS})"
        ),
    )
    parser.add_argument(
        "--reviewed-dispositions",
        default=str(DEFAULT_REVIEWED_DISPOSITIONS),
        help=(
            "optional TSV of manually reviewed final dispositions "
            f"(default: {DEFAULT_REVIEWED_DISPOSITIONS})"
        ),
    )
    parser.add_argument(
        "--summary",
        action="store_true",
        help="print disposition counts instead of row details",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    rows = build_rows(args)
    if args.summary:
        emit_summary(rows)
    else:
        emit_tsv(rows)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3

import pathlib


ROOT = pathlib.Path(__file__).resolve().parents[1]
README = ROOT / "validation" / "README.md"
TRACKER = ROOT / "validation" / "INGEST_TRACKER.md"


def parse_candidates():
    rows = []
    for line in README.read_text().splitlines():
        if not line.startswith("| V"):
            continue
        cells = [cell.strip() for cell in line.strip().split("|")[1:-1]]
        if len(cells) < 5:
            continue
        candidate_id, focus, feature, kind = cells[:4]
        rows.append(
            {
                "id": candidate_id,
                "focus": focus,
                "feature": feature,
                "kind": kind,
            }
        )
    if not rows:
        raise SystemExit("no validation candidates found in validation/README.md")
    return rows


def render_tracker(rows):
    lines = [
        "# Validation Ingest Tracker",
        "",
        "This tracker is generated from the candidate matrix in",
        "[`validation/README.md`](./README.md) and then updated manually as items are",
        "ingested into the `pa*` regression suites.",
        "",
        "Regenerate the initial skeleton with:",
        "",
        "```sh",
        "python3 scripts/generate_validation_ingest_tracker.py",
        "```",
        "",
        "| ID | Kind | N3485 focus | Feature | Target PA | Status | Regression | Commit | Notes |",
        "|---|---|---|---|---|---|---|---|---|",
    ]
    for row in rows:
        lines.append(
            f"| {row['id']} | {row['kind']} | {row['focus']} | "
            f"{row['feature']} |  | todo |  |  |  |"
        )
    lines.append("")
    return "\n".join(lines)


def main():
    TRACKER.write_text(render_tracker(parse_candidates()))


if __name__ == "__main__":
    main()

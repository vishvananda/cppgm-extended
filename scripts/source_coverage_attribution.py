#!/usr/bin/env python3
import argparse
import csv
import json
import os
import re
import sys


LINE_COUNT_RE = re.compile(r"^\s*([0-9]+)\|([^|]*)\|")


def pa_sort_key(path):
    name = os.path.basename(os.path.dirname(path))
    match = re.match(r"pa([0-9]+)$", name)
    if match:
        return (0, int(match.group(1)), name)
    return (1, 0, name)


def pa_name_sort_key(name):
    match = re.match(r"pa([0-9]+)$", name)
    if match:
        return (0, int(match.group(1)), name)
    return (1, 0, name)


def display_path(filename, repo_root):
    filename = os.path.abspath(filename)
    try:
        rel = os.path.relpath(filename, repo_root)
    except ValueError:
        return filename
    if rel == "." or rel.startswith(".."):
        return filename
    return rel


def line_counts_from_file(file_entry):
    segments = file_entry.get("segments", [])
    counts = {}
    active = False
    current_count = 0

    for index, segment in enumerate(segments):
        if len(segment) < 4:
            continue

        line = int(segment[0])
        has_count = bool(segment[3])
        is_gap_region = bool(segment[5]) if len(segment) > 5 else False

        if has_count and not is_gap_region:
            active = True
            current_count = int(segment[2])
        else:
            active = False

        if not active:
            continue

        if index + 1 < len(segments):
            next_line = int(segments[index + 1][0])
        else:
            next_line = line
        end_line = line if next_line <= line else next_line - 1

        for covered_line in range(line, end_line + 1):
            counts[covered_line] = max(counts.get(covered_line, 0), current_count)

    return counts


def load_coverage_counts(path, repo_root):
    with open(path) as handle:
        payload = json.load(handle)

    merged = {}
    for data in payload.get("data", []):
        for file_entry in data.get("files", []):
            filename = display_path(file_entry.get("filename", ""), repo_root)
            if not filename:
                continue
            file_counts = merged.setdefault(filename, {})
            for line, count in line_counts_from_file(file_entry).items():
                file_counts[line] = max(file_counts.get(line, 0), count)
    return merged


def count_field_is_hit(field):
    value = field.strip()
    if not value:
        return None
    if set(value) == set("#"):
        return False
    return value != "0"


def load_line_count_text(path, repo_root):
    counts = {}
    current_file = None

    with open(path) as handle:
        for raw_line in handle:
            line = raw_line.rstrip("\n")
            match = LINE_COUNT_RE.match(line)
            if match:
                if current_file is None:
                    continue
                hit = count_field_is_hit(match.group(2))
                if hit is None:
                    continue
                counts.setdefault(current_file, {})[int(match.group(1))] = 1 if hit else 0
                continue

            if line.startswith("/") and line.endswith(":"):
                current_file = display_path(line[:-1], repo_root)

    return counts


def load_counts(json_path, text_path, repo_root):
    if text_path and os.path.isfile(text_path):
        return load_line_count_text(text_path, repo_root)
    return load_coverage_counts(json_path, repo_root)


def discover_pa_jsons(by_pa_dir):
    jsons = []
    if not by_pa_dir or not os.path.isdir(by_pa_dir):
        return jsons
    for name in os.listdir(by_pa_dir):
        path = os.path.join(by_pa_dir, name, "coverage.json")
        if os.path.isfile(path):
            jsons.append(path)
    return sorted(jsons, key=pa_sort_key)


def pa_text_for_json(json_path):
    text_path = os.path.join(os.path.dirname(json_path), "line-counts.txt")
    if os.path.isfile(text_path):
        return text_path
    return None


def grouped_ranges(lines):
    ranges = []
    start = None
    previous = None

    for line in sorted(lines):
        if start is None:
            start = line
            previous = line
            continue
        if line == previous + 1:
            previous = line
            continue
        ranges.append((start, previous))
        start = line
        previous = line

    if start is not None:
        ranges.append((start, previous))
    return ranges


def write_review_queue(unhit_by_file, out_review):
    with open(out_review, "w") as handle:
        handle.write("# Coverage Review Queue\n\n")
        handle.write(
            "Classify each uncovered range as `test-needed`, `remove`, "
            "`config-gap`, `bug`, `defensive`, or `artifact`.\n\n"
        )
        for filename in sorted(unhit_by_file):
            handle.write("## {}\n\n".format(filename))
            for start, end in grouped_ranges(unhit_by_file[filename]):
                if start == end:
                    location = "{}:{}".format(filename, start)
                else:
                    location = "{}:{}-{}".format(filename, start, end)
                handle.write("- `{}` - unclassified\n".format(location))
            handle.write("\n")


def write_outputs(
    full_counts,
    pa_counts,
    out_csv,
    out_json,
    out_summary,
    out_unhit,
    out_review,
):
    all_locations = set()
    for filename, lines in full_counts.items():
        for line in lines:
            all_locations.add((filename, line))
    for counts in pa_counts.values():
        for filename, lines in counts.items():
            for line in lines:
                all_locations.add((filename, line))

    rows = []
    hit_lines = 0
    unhit_lines = 0
    per_pa_hits = {pa: 0 for pa in pa_counts}
    files = set()
    unhit_by_file = {}

    for filename, line in sorted(all_locations):
        files.add(filename)
        hit_pas = []
        for pa, counts in pa_counts.items():
            if counts.get(filename, {}).get(line, 0) > 0:
                hit_pas.append(pa)
                per_pa_hits[pa] += 1

        hit_any = full_counts.get(filename, {}).get(line, 0) > 0 or bool(hit_pas)
        if hit_any:
            hit_lines += 1
        else:
            unhit_lines += 1
            unhit_by_file.setdefault(filename, []).append(line)

        rows.append(
            {
                "file": filename,
                "line": line,
                "hit_any": hit_any,
                "hit_pas": hit_pas,
            }
        )

    with open(out_csv, "w", newline="") as handle:
        writer = csv.writer(handle)
        writer.writerow(["file", "line", "hit_any", "hit_pas"])
        for row in rows:
            writer.writerow(
                [
                    row["file"],
                    row["line"],
                    "1" if row["hit_any"] else "0",
                    " ".join(row["hit_pas"]),
                ]
            )

    with open(out_json, "w") as handle:
        json.dump({"lines": rows}, handle, indent=2, sort_keys=True)
        handle.write("\n")

    with open(out_unhit, "w") as handle:
        for row in rows:
            if not row["hit_any"]:
                handle.write("{}:{}\n".format(row["file"], row["line"]))

    write_review_queue(unhit_by_file, out_review)

    with open(out_summary, "w") as handle:
        handle.write("files={}\n".format(len(files)))
        handle.write("executable_lines={}\n".format(len(rows)))
        handle.write("hit_lines={}\n".format(hit_lines))
        handle.write("unhit_lines={}\n".format(unhit_lines))
        handle.write("pa_count={}\n".format(len(pa_counts)))
        for pa in sorted(per_pa_hits, key=pa_name_sort_key):
            handle.write("{}_hit_lines={}\n".format(pa, per_pa_hits[pa]))


def main(argv):
    parser = argparse.ArgumentParser(
        description="Build a source-line to assignment-hit matrix from llvm-cov export JSON."
    )
    parser.add_argument("--repo-root", required=True)
    parser.add_argument("--full-json", required=True)
    parser.add_argument("--full-text")
    parser.add_argument("--by-pa-dir", required=True)
    parser.add_argument("--out-csv", required=True)
    parser.add_argument("--out-json", required=True)
    parser.add_argument("--out-summary", required=True)
    parser.add_argument("--out-unhit", required=True)
    parser.add_argument("--out-review", required=True)
    args = parser.parse_args(argv)

    repo_root = os.path.abspath(args.repo_root)
    full_counts = load_counts(args.full_json, args.full_text, repo_root)
    pa_counts = {}
    for path in discover_pa_jsons(args.by_pa_dir):
        pa = os.path.basename(os.path.dirname(path))
        pa_counts[pa] = load_counts(path, pa_text_for_json(path), repo_root)

    write_outputs(
        full_counts,
        pa_counts,
        args.out_csv,
        args.out_json,
        args.out_summary,
        args.out_unhit,
        args.out_review,
    )
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))

#!/usr/bin/env python3

import hashlib
import json
import pathlib
from typing import Dict, Mapping, Optional, Tuple


def _sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def _require_sha256(entry: Mapping[str, object], field: str, entry_id: str) -> str:
    value = entry.get(field)
    if (not isinstance(value, str) or len(value) != 64 or
            any(char not in "0123456789abcdef" for char in value)):
        raise ValueError(f"adjudication {entry_id} has invalid {field}")
    return value


def load_witness_adjudications(path: pathlib.Path) -> Dict[str, dict]:
    manifest = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(manifest, dict) or manifest.get("schema_version") != 1:
        raise ValueError(f"unsupported witness adjudication schema in {path}")
    entries = manifest.get("adjudications")
    if not isinstance(entries, list):
        raise ValueError(f"witness adjudication manifest has no adjudication list: {path}")

    by_test: Dict[str, dict] = {}
    for entry in entries:
        if not isinstance(entry, dict):
            raise ValueError(f"invalid witness adjudication entry in {path}")
        entry_id = entry.get("id")
        test = entry.get("test")
        reason = entry.get("reason")
        if not isinstance(entry_id, str) or not entry_id:
            raise ValueError(f"witness adjudication entry lacks id in {path}")
        if not isinstance(test, str) or not test:
            raise ValueError(f"adjudication {entry_id} lacks test")
        if not isinstance(reason, str) or not reason:
            raise ValueError(f"adjudication {entry_id} lacks reason")
        _require_sha256(entry, "source_sha256", entry_id)
        _require_sha256(entry, "raw_clang_witness_sha256", entry_id)
        _require_sha256(entry, "adjudicated_witness_sha256", entry_id)
        rewrites = entry.get("rewrites")
        if not isinstance(rewrites, list) or not rewrites:
            raise ValueError(f"adjudication {entry_id} has no rewrites")
        for rewrite in rewrites:
            if not isinstance(rewrite, dict):
                raise ValueError(f"adjudication {entry_id} has an invalid rewrite")
            before = rewrite.get("before")
            after = rewrite.get("after")
            if not isinstance(before, str) or not before:
                raise ValueError(f"adjudication {entry_id} has an empty rewrite input")
            if not isinstance(after, str):
                raise ValueError(f"adjudication {entry_id} has an invalid rewrite output")
        if test in by_test:
            raise ValueError(f"duplicate witness adjudication for {test}")
        by_test[test] = entry
    return by_test


def apply_witness_adjudication(
    repo_root: pathlib.Path,
    test: pathlib.Path,
    raw_clang_text: str,
    adjudications: Mapping[str, dict],
) -> Tuple[str, Optional[str]]:
    repo_root = repo_root.resolve()
    test = test.resolve()
    relative_test = test.relative_to(repo_root).as_posix()
    entry = adjudications.get(relative_test)
    if entry is None:
        return raw_clang_text, None

    entry_id = entry["id"]
    observed_source_hash = _sha256_bytes(test.read_bytes())
    expected_source_hash = entry["source_sha256"]
    if observed_source_hash != expected_source_hash:
        raise ValueError(
            f"adjudication {entry_id} source_sha256 changed: "
            f"expected {expected_source_hash}, observed {observed_source_hash}"
        )

    observed_raw_hash = _sha256_bytes(raw_clang_text.encode("utf-8"))
    expected_raw_hash = entry["raw_clang_witness_sha256"]
    if observed_raw_hash != expected_raw_hash:
        raise ValueError(
            f"adjudication {entry_id} raw_clang_witness_sha256 changed: "
            f"expected {expected_raw_hash}, observed {observed_raw_hash}"
        )

    adjudicated_text = raw_clang_text
    for index, rewrite in enumerate(entry["rewrites"], start=1):
        before = rewrite["before"]
        count = adjudicated_text.count(before)
        if count != 1:
            raise ValueError(
                f"adjudication {entry_id} rewrite {index} matched {count} times"
            )
        adjudicated_text = adjudicated_text.replace(before, rewrite["after"], 1)

    observed_adjudicated_hash = _sha256_bytes(adjudicated_text.encode("utf-8"))
    expected_adjudicated_hash = entry["adjudicated_witness_sha256"]
    if observed_adjudicated_hash != expected_adjudicated_hash:
        raise ValueError(
            f"adjudication {entry_id} produced unexpected witness: "
            f"expected {expected_adjudicated_hash}, observed {observed_adjudicated_hash}"
        )
    return adjudicated_text, entry_id

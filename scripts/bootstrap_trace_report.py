#!/usr/bin/env python3

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shlex
import shutil
import subprocess
import sys
import tempfile
import time
from typing import Dict, Iterable, List, Optional, Sequence, Tuple


def parse_args():
    parser = argparse.ArgumentParser(
        description="Bootstrap frontier unresolved-symbol triage and consumer/provider symbol diff"
    )
    parser.add_argument("--repo-root", default=".")
    parser.add_argument("--cluster", default="")
    parser.add_argument("--link-stdout", default="")
    parser.add_argument("--focus", action="append", default=[],
                        help="substring filter applied to raw or demangled symbol names")
    parser.add_argument("--consumer", action="append", default=[],
                        help="source file to compile and inspect for unresolved symbols")
    parser.add_argument("--provider", action="append", default=[],
                        help="source file to compile and inspect for defined symbols")
    parser.add_argument("--consumer-object", action="append", default=[],
                        help="prebuilt object file to inspect for unresolved symbols")
    parser.add_argument("--provider-object", action="append", default=[],
                        help="prebuilt object file to inspect for defined symbols")
    parser.add_argument("--compiler", default="./dev/cppgm++")
    parser.add_argument("--host-cxx",
                        default=os.environ.get("CPPGM_HOST_CXX",
                                               "/usr/local/opt/llvm/bin/clang++"))
    parser.add_argument("--include-dir", default="dev/src")
    parser.add_argument("--nm", default=os.environ.get("CPPGM_HOST_NM", "nm"))
    parser.add_argument("--cxxfilt", default=os.environ.get("CPPGM_HOST_CXXFILT", "c++filt"))
    parser.add_argument("--keep-temp", action="store_true")
    parser.add_argument("--cluster-probe", action="store_true",
                        help="reuse preserved cluster objects for focused consumer/provider diff")
    parser.add_argument("--trace-source", action="append", default=[],
                        help="source file to compile with focused parser trace presets")
    parser.add_argument("--trace-stderr", action="append", default=[],
                        help="raw stderr/trace file to analyze without recompiling")
    parser.add_argument("--pipeline-source", action="append", default=[],
                        help="source file to analyze across semantic output and LowIR symbol stages")
    parser.add_argument("--preset", default="",
                        choices=["template-lifecycle", "output-lifecycle",
                                 "linkage", "full-link-root-cause"],
                        help="trace preset to use with --trace-source")
    parser.add_argument("--file", dest="trace_file_filter", default="",
                        help="optional CPPGM_TRACE_FILE substring for focused trace reruns")
    parser.add_argument("--trace-limit", type=int, default=1200)
    parser.add_argument("--json", dest="json_output", action="store_true",
                        help="emit structured JSON instead of human-readable text")
    parser.add_argument("--write-prefix", default="",
                        help="persist concise text, detailed text, and JSON outputs at this prefix")
    parser.add_argument("--top", type=int, default=12)
    return parser.parse_args()


def run(cmd: Sequence[str], cwd: Path, env: Optional[Dict[str, str]] = None) -> subprocess.CompletedProcess:
    return subprocess.run(
        list(cmd),
        cwd=str(cwd),
        env=env,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )


def read_text(path: Path) -> str:
    return path.read_text()


def read_cli_text(path_text: str) -> Tuple[str, str]:
    if path_text == "-":
        return sys.stdin.read(), "<stdin>"
    path = Path(path_text).resolve()
    return path.read_text(), str(path)


def write_status(path: Optional[Path], **fields: object) -> None:
    if path is None:
        return
    lines = [f"{key}={value}" for key, value in fields.items()]
    path.write_text("\n".join(lines) + "\n")


def file_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        while True:
            chunk = handle.read(65536)
            if not chunk:
                break
            digest.update(chunk)
    return digest.hexdigest()


def resolve_tool(explicit: str, *fallbacks: str) -> str:
    candidates = []
    if explicit:
        candidates.append(explicit)
    candidates.extend(fallbacks)
    for candidate in candidates:
        resolved = shutil.which(candidate)
        if resolved:
            return resolved
    raise SystemExit(f"Unable to resolve tool from: {', '.join(candidates)}")


_REPO_CANDIDATE_CACHE: Dict[Tuple[str, str], List[str]] = {}


def demangle_symbols(symbols: Iterable[str], cxxfilt: str) -> Dict[str, str]:
    ordered = [symbol for symbol in symbols if symbol]
    if not ordered:
        return {}
    demangle_input = "".join(f"{symbol}\n" for symbol in ordered)
    proc = subprocess.run(
        [cxxfilt],
        input=demangle_input,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    if proc.returncode != 0:
        return {symbol: symbol for symbol in ordered}
    lines = proc.stdout.splitlines()
    result = {}
    for i, symbol in enumerate(ordered):
        result[symbol] = lines[i] if i < len(lines) and lines[i] else symbol
    return result


class SymbolRef:
    def __init__(self, raw: str):
        self.raw = raw
        self.demangled = raw
        self.referencers: List[str] = []


def parse_linker_undefineds(text: str) -> List[SymbolRef]:
    results: List[SymbolRef] = []
    by_raw: Dict[str, SymbolRef] = {}
    current: Optional[SymbolRef] = None
    pending_context_object: Optional[str] = None

    def object_basename(text_value: str) -> str:
        return Path(text_value.strip()).name

    def ensure_symbol(raw: str) -> SymbolRef:
        symbol = by_raw.get(raw)
        if symbol is None:
            symbol = SymbolRef(raw)
            by_raw[raw] = symbol
            results.append(symbol)
        return symbol

    def add_referencer(symbol: Optional[SymbolRef], referencer: Optional[str]) -> None:
        if symbol is None or not referencer:
            return
        referencer = object_basename(referencer)
        if referencer and referencer not in symbol.referencers:
            symbol.referencers.append(referencer)

    for line in text.splitlines():
        match = re.match(r'\s*"([^"]+)", referenced from:', line)
        if match:
            current = ensure_symbol(match.group(1))
            continue

        ref_match = re.search(r' in ([^ ]+\.o)\s*$', line)
        if ref_match:
            add_referencer(current, ref_match.group(1))
            continue

        context_match = re.search(r'([^ :]+\.o):(?: in function .*)?$', line)
        if context_match:
            pending_context_object = context_match.group(1)

        gnu_match = re.search(r"undefined reference to [`'\"]?(.+?)[`'\"]?$", line)
        if gnu_match:
            current = ensure_symbol(gnu_match.group(1))
            inline_object = re.search(r'([^ :]+\.o):', line)
            add_referencer(current,
                           inline_object.group(1) if inline_object else pending_context_object)
            continue

        lld_match = re.search(r'undefined symbol: (.+)$', line)
        if lld_match:
            current = ensure_symbol(lld_match.group(1).strip())
            continue

        if line.lstrip().startswith(">>>"):
            object_match = re.search(r'([^\s:()]+\.o)(?=[:() ]|$)', line)
            if object_match:
                add_referencer(current, object_match.group(1))
            continue

        if line.strip():
            current = None
    return results


def parse_linker_duplicates(text: str) -> List[SymbolRef]:
    results: List[SymbolRef] = []
    by_raw: Dict[str, SymbolRef] = {}
    current: Optional[SymbolRef] = None

    def object_basename(text_value: str) -> str:
        return Path(text_value.strip()).name

    def ensure_symbol(raw: str) -> SymbolRef:
        symbol = by_raw.get(raw)
        if symbol is None:
            symbol = SymbolRef(raw)
            by_raw[raw] = symbol
            results.append(symbol)
        return symbol

    def add_object(symbol: Optional[SymbolRef], object_text: Optional[str]) -> None:
        if symbol is None or not object_text:
            return
        referencer = object_basename(object_text)
        if referencer and referencer not in symbol.referencers:
            symbol.referencers.append(referencer)

    for line in text.splitlines():
        match = re.match(r"\s*duplicate symbol '(.+)' in:\s*$", line)
        if match:
            current = ensure_symbol(match.group(1))
            continue

        if current is not None:
            object_match = re.match(r"\s+(.+?\.o)\s*$", line)
            if object_match:
                add_object(current, object_match.group(1))
                continue

        if line.strip():
            current = None
    return results


def link_failure_stage(data: dict) -> dict:
    stages = data.get("stages", [])
    if not stages:
        raise SystemExit("No stages found")
    for stage in reversed(stages):
        if stage.get("stage") == "link" and stage.get("returncode") != 0:
            return stage
    raise SystemExit(f"report does not contain a failed link stage (result={data.get('result', 'unknown')})")


def load_frontier_undefineds(path: Path) -> Tuple[dict, List[SymbolRef]]:
    data, undefineds, _ = load_frontier_link_issues(path)
    return data, undefineds


def load_frontier_link_issues(path: Path) -> Tuple[dict, List[SymbolRef], List[SymbolRef]]:
    data = json.loads(path.read_text())
    try:
        last = link_failure_stage(data)
    except SystemExit as exc:
        raise SystemExit(f"{path}: {exc}") from None
    stdout = last.get("stdout", "")
    undefineds = parse_linker_undefineds(stdout)
    duplicates = parse_linker_duplicates(stdout)
    return data, undefineds, duplicates


def cluster_object_map(cluster_data: dict) -> Dict[str, Path]:
    out: Dict[str, Path] = {}
    for stage in cluster_data.get("stages", []):
        if stage.get("stage") != "compile":
            continue
        source = stage.get("source")
        obj = stage.get("object")
        if source and obj:
            out[source] = Path(obj)
    return out


def cluster_object_basename_map(cluster_data: dict) -> Dict[str, Path]:
    out: Dict[str, Path] = {}
    for stage in cluster_data.get("stages", []):
        if stage.get("stage") != "compile":
            continue
        obj = stage.get("object")
        if obj:
            path = Path(obj)
            out[path.name] = path
    return out


def cluster_object_source_basename_map(cluster_data: dict) -> Dict[str, str]:
    out: Dict[str, str] = {}
    for stage in cluster_data.get("stages", []):
        if stage.get("stage") != "compile":
            continue
        source = stage.get("source")
        obj = stage.get("object")
        if source and obj:
            out[Path(obj).name] = source
    return out


def base_name(demangled: str) -> str:
    return demangled.split("(", 1)[0].strip()


def classify_symbol(demangled: str) -> str:
    if demangled.startswith("std::__1::") or demangled.startswith("__ZTVNSt3__1"):
        return "hosted-libc++/ABI"
    if demangled.startswith("__cppgm_") or demangled.startswith("cppgm_"):
        return "compiler-runtime"
    if demangled.startswith("__cxa_") or demangled.startswith("__gxx_"):
        return "host-EH/runtime"
    if "::" in demangled:
        return "compiler/provider-or-signature-drift"
    return "other"


def symbol_matches_focus(raw: str, demangled: str, focuses: Sequence[str]) -> bool:
    if not focuses:
        return True
    lowered_raw = raw.lower()
    lowered_demangled = demangled.lower()
    for focus in focuses:
        needle = focus.lower()
        if needle in lowered_raw or needle in lowered_demangled:
            return True
    return False


def find_repo_candidates(repo_root: Path, demangled: str, limit: int = 8) -> List[str]:
    name = base_name(demangled).split("::")[-1]
    if not name:
        return []
    cache_key = (str(repo_root), name)
    if cache_key not in _REPO_CANDIDATE_CACHE:
        proc = run(
            ["rg", "-n", "-l", "-F", name, "dev", "scripts"],
            repo_root,
        )
        if proc.returncode not in (0, 1):
            _REPO_CANDIDATE_CACHE[cache_key] = []
        else:
            _REPO_CANDIDATE_CACHE[cache_key] = [
                line.strip() for line in proc.stdout.splitlines() if line.strip()
            ]
    results = _REPO_CANDIDATE_CACHE[cache_key]
    return results[:limit]


def find_repo_source_candidates(repo_root: Path, demangled: str, limit: int = 12) -> List[str]:
    return [
        candidate for candidate in find_repo_candidates(repo_root, demangled, limit=limit * 2)
        if candidate.endswith(".cpp") or candidate.endswith(".cc") or candidate.endswith(".cxx")
    ][:limit]


def trim(s: str, limit: int = 140) -> str:
    return s if len(s) <= limit else s[:limit - 3] + "..."


def short_path(repo_root: Path, value: str) -> str:
    path = Path(value)
    try:
        return str(path.resolve().relative_to(repo_root))
    except Exception:
        return path.name if path.name else value


def shell_join(parts: Sequence[str]) -> str:
    return " ".join(shlex.quote(part) for part in parts)


def focus_token_for_symbol(demangled: str) -> str:
    base = base_name(demangled)
    if "::" in base:
        tail = base.split("::")[-1]
        if tail:
            return tail
    return base


def guess_trace_preset(family: str, demangled: str) -> str:
    lowered = demangled.lower()
    if "vtable" in lowered or "typeinfo" in lowered or "__cxa_" in lowered:
        return "full-link-root-cause"
    if family == "compiler/provider-or-signature-drift":
        return "linkage"
    if family == "compiler-runtime":
        return "full-link-root-cause"
    if family == "hosted-libc++/ABI":
        return "output-lifecycle"
    return "full-link-root-cause"


def frontier_candidate_score(item: SymbolRef,
                             family: str,
                             provider_sources: Sequence[str],
                             consumer_sources: Sequence[str]) -> Tuple[int, int, int, str]:
    score = 0
    if family == "compiler/provider-or-signature-drift":
        score += 40
    elif family == "compiler-runtime":
        score += 30
    elif family == "hosted-libc++/ABI":
        score += 20
    elif family == "host-EH/runtime":
        score += 15
    score += min(len(provider_sources), 3) * 4
    score += min(len(consumer_sources), 3) * 3
    score += min(len(set(item.referencers)), 4) * 2
    if "std::__1::" in item.demangled and family == "compiler/provider-or-signature-drift":
        score += 3
    return (-score, -len(provider_sources), -len(consumer_sources), item.demangled)


def frontier_integration_data(repo_root: Path,
                              cluster_path: Path,
                              cluster_data: dict,
                              undefineds: List[SymbolRef],
                              duplicates: List[SymbolRef],
                              focuses: Sequence[str],
                              top: int) -> dict:
    object_to_source = cluster_object_source_basename_map(cluster_data)
    build_dir_text = cluster_data.get("build_dir", "")
    build_dir = Path(build_dir_text) if build_dir_text else None
    build_dir_exists = build_dir.exists() if build_dir else False
    candidates = []

    if duplicates:
        filtered = [item for item in duplicates if symbol_matches_focus(item.raw, item.demangled, focuses)]
        display_items = filtered if focuses else duplicates
        for item in display_items:
            family = classify_symbol(item.demangled)
            duplicate_sources = []
            for referencer in sorted(set(item.referencers)):
                source = object_to_source.get(referencer)
                if source and source not in duplicate_sources:
                    duplicate_sources.append(source)
            trace_source = duplicate_sources[0] if duplicate_sources else ""
            preset = guess_trace_preset(family, item.demangled)
            trace_command = ""
            if trace_source:
                trace_command = shell_join([
                    "python3",
                    "scripts/bootstrap_trace_report.py",
                    "--repo-root", str(repo_root),
                    "--trace-source", trace_source,
                    "--preset", preset,
                    "--focus", focus_token_for_symbol(item.demangled),
                    "--file", Path(trace_source).name,
                ])
            candidates.append({
                "raw": item.raw,
                "demangled": item.demangled,
                "family": family,
                "duplicate_objects": sorted(set(item.referencers))[:8],
                "duplicate_sources": duplicate_sources[:6],
                "suggested_trace_source": trace_source,
                "suggested_preset": preset,
                "trace_command": trace_command,
                "rerun_frontier_command": shell_join([
                    "python3",
                    "scripts/report_bootstrap_frontier.py",
                    "--repo-root", str(repo_root),
                    "--output-prefix",
                    build_dir_text[:-6] if build_dir_text.endswith(".build") else
                    str(cluster_path).rsplit(".json", 1)[0],
                    "--compiler", cluster_data.get("compiler", "./dev/cppgm++"),
                    "--host-cxx", cluster_data.get("host_cxx",
                                                   "/usr/local/opt/llvm/bin/clang++"),
                    "--jobs", str(cluster_data.get("jobs", 1)),
                    "--frontend", cluster_data.get("frontend", "dev/cppgm++.cpp"),
                ]),
                "_sort_key": (
                    -({"compiler/provider-or-signature-drift": 40,
                       "compiler-runtime": 30,
                       "hosted-libc++/ABI": 20,
                       "host-EH/runtime": 15}.get(family, 10) +
                      min(len(duplicate_sources), 4) * 4 +
                      min(len(set(item.referencers)), 6) * 3),
                    -len(duplicate_sources),
                    -len(set(item.referencers)),
                    item.demangled,
                ),
            })

        candidates.sort(key=lambda item: item["_sort_key"])
        for item in candidates:
            del item["_sort_key"]

        return {
            "cluster_json": str(cluster_path),
            "build_dir": build_dir_text,
            "build_dir_exists": build_dir_exists,
            "candidate_count": len(candidates),
            "issue_kind": "duplicate",
            "candidates": candidates[:max(top, 1)],
            "best_candidate": candidates[0] if candidates else None,
        }

    filtered = [item for item in undefineds if symbol_matches_focus(item.raw, item.demangled, focuses)]
    display_items = filtered if focuses else undefineds
    for item in display_items:
        family = classify_symbol(item.demangled)
        provider_sources = find_repo_source_candidates(repo_root, item.demangled)
        consumer_sources = []
        for referencer in sorted(set(item.referencers)):
            source = object_to_source.get(referencer)
            if source and source not in consumer_sources:
                consumer_sources.append(source)

        focus = focus_token_for_symbol(item.demangled)
        preset = guess_trace_preset(family, item.demangled)
        trace_source = provider_sources[0] if provider_sources else (
            consumer_sources[0] if consumer_sources else "")
        provider_diff_command = ""
        if build_dir_exists:
            provider_diff_command = shell_join([
                "python3",
                "scripts/bootstrap_trace_report.py",
                "--repo-root", str(repo_root),
                "--cluster", str(cluster_path),
                "--cluster-probe",
                "--focus", focus,
            ])

        trace_command = ""
        if trace_source:
            trace_command = shell_join([
                "python3",
                "scripts/bootstrap_trace_report.py",
                "--repo-root", str(repo_root),
                "--trace-source", trace_source,
                "--preset", preset,
                "--focus", focus,
                "--file", Path(trace_source).name,
            ])

        candidates.append({
            "raw": item.raw,
            "demangled": item.demangled,
            "family": family,
            "focus": focus,
            "consumer_sources": consumer_sources[:6],
            "provider_sources": provider_sources[:6],
            "suggested_trace_source": trace_source,
            "suggested_preset": preset,
            "provider_diff_command": provider_diff_command,
            "trace_command": trace_command,
            "rerun_frontier_command": shell_join([
                "python3",
                "scripts/report_bootstrap_frontier.py",
                "--repo-root", str(repo_root),
                "--output-prefix",
                build_dir_text[:-6] if build_dir_text.endswith(".build") else
                str(cluster_path).rsplit(".json", 1)[0],
                "--compiler", cluster_data.get("compiler", "./dev/cppgm++"),
                "--host-cxx", cluster_data.get("host_cxx",
                                               "/usr/local/opt/llvm/bin/clang++"),
                "--jobs", str(cluster_data.get("jobs", 1)),
                "--frontend", cluster_data.get("frontend", "dev/cppgm++.cpp"),
            ]),
            "_sort_key": frontier_candidate_score(item, family, provider_sources, consumer_sources),
        })

    candidates.sort(key=lambda item: item["_sort_key"])
    for item in candidates:
        del item["_sort_key"]

    return {
        "cluster_json": str(cluster_path),
        "build_dir": build_dir_text,
        "build_dir_exists": build_dir_exists,
        "candidate_count": len(candidates),
        "issue_kind": "undefined",
        "candidates": candidates[:max(top, 1)],
        "best_candidate": candidates[0] if candidates else None,
    }


def render_frontier_integration(data: dict) -> str:
    lines = []
    lines.append("Frontier Integration")
    lines.append(f"- cluster_json: {data.get('cluster_json', '')}")
    if data.get("build_dir"):
        lines.append(f"- build_dir: {data['build_dir']}")
        lines.append(f"- build_dir_exists: {'yes' if data.get('build_dir_exists') else 'no'}")

    best = data.get("best_candidate")
    if not best:
        lines.append("- no candidate symbols available")
        return "\n".join(lines)

    lines.append("")
    lines.append("Suggested Next Step")
    lines.append(f"- symbol: {best['demangled']}")
    lines.append(f"- family: {best['family']}")
    if data.get("issue_kind") == "duplicate":
        if best.get("duplicate_sources"):
            lines.append(f"- likely_sources: {', '.join(best['duplicate_sources'][:3])}")
        if best.get("duplicate_objects"):
            lines.append(f"- duplicate_in: {', '.join(best['duplicate_objects'][:6])}")
    else:
        if best.get("consumer_sources"):
            lines.append(f"- likely_consumer: {', '.join(best['consumer_sources'][:3])}")
        if best.get("provider_sources"):
            lines.append(f"- likely_provider: {', '.join(best['provider_sources'][:3])}")
    if best.get("suggested_trace_source"):
        lines.append(f"- trace_source: {best['suggested_trace_source']}")
    lines.append(f"- trace_preset: {best['suggested_preset']}")
    if best.get("provider_diff_command"):
        lines.append(f"- provider_diff: {best['provider_diff_command']}")
    elif best.get("rerun_frontier_command"):
        lines.append(f"- rebuild_frontier: {best['rerun_frontier_command']}")
    if best.get("trace_command"):
        lines.append(f"- trace_rerun: {best['trace_command']}")

    if len(data.get("candidates", [])) > 1:
        lines.append("")
        lines.append("Other Candidates")
        for item in data["candidates"][1:]:
            lines.append(f"- {item['demangled']} [{item['family']}]")
    return "\n".join(lines)


def frontier_summary_data(repo_root: Path,
                          cluster_data: dict,
                          undefineds: List[SymbolRef],
                          duplicates: List[SymbolRef],
                          focuses: Sequence[str],
                          top: int,
                          include_repo_candidates: bool) -> dict:
    items = duplicates if duplicates else undefineds
    family_counts: Dict[str, int] = {}
    for item in items:
        family = classify_symbol(item.demangled)
        family_counts[family] = family_counts.get(family, 0) + 1

    focused = [item for item in items if symbol_matches_focus(item.raw, item.demangled, focuses)]
    displayed = focused if focuses else focused[:top]
    symbols = []
    for item in displayed[:top]:
        entry = {
            "raw": item.raw,
            "demangled": item.demangled,
            "family": classify_symbol(item.demangled),
            "referencers": sorted(set(item.referencers))[:8],
        }
        if include_repo_candidates:
            candidates = find_repo_candidates(repo_root, item.demangled)
            if candidates:
                entry["repo_candidates"] = candidates
        symbols.append(entry)

    return {
        "result": cluster_data.get("result"),
        "active_frontier": cluster_data.get("active_frontier"),
        "undefined_symbol_count": len(undefineds),
        "duplicate_symbol_count": len(duplicates),
        "primary_issue_kind": "duplicate" if duplicates else "undefined",
        "family_counts": dict(sorted(family_counts.items())),
        "focused": bool(focuses),
        "symbols": symbols,
    }


def render_frontier_summary(repo_root: Path,
                            cluster_data: dict,
                            undefineds: List[SymbolRef],
                            duplicates: List[SymbolRef],
                            focuses: Sequence[str],
                            top: int,
                            verbose: bool) -> str:
    data = frontier_summary_data(repo_root,
                                 cluster_data,
                                 undefineds,
                                 duplicates,
                                 focuses,
                                 top,
                                 verbose)
    lines = []
    lines.append("Frontier Summary")
    lines.append(f"result: {data.get('result')}")
    lines.append(f"active_frontier: {data.get('active_frontier')}")
    lines.append(f"undefined_symbols: {data.get('undefined_symbol_count')}")
    lines.append(f"duplicate_symbols: {data.get('duplicate_symbol_count')}")
    if verbose:
        lines.append("")
        lines.append("Duplicate Families" if data.get("primary_issue_kind") == "duplicate"
                     else "Undefined Families")
        for family, count in sorted(data["family_counts"].items(), key=lambda item: (-item[1], item[0])):
            lines.append(f"- {family}: {count}")

    if focuses:
        lines.append("")
        lines.append(f"Focused Symbols ({len(data['symbols'])})")
    else:
        lines.append("")
        lines.append(
            f"Top Duplicate Symbols ({len(data['symbols'])})"
            if data.get("primary_issue_kind") == "duplicate"
            else f"Top Symbols ({len(data['symbols'])})"
        )

    for item in data["symbols"]:
        lines.append(f"- {item['demangled']}")
        lines.append(f"  family: {item['family']}")
        if item.get("referencers"):
            lines.append(
                f"  duplicate_in: {', '.join(item['referencers'])}"
                if data.get("primary_issue_kind") == "duplicate"
                else f"  referenced_from: {', '.join(item['referencers'])}"
            )
        if verbose:
            lines.append(f"  raw: {item['raw']}")
            if item.get("repo_candidates"):
                lines.append(f"  repo_candidates: {', '.join(item['repo_candidates'])}")
    return "\n".join(lines)


def infer_cluster_probe_objects(repo_root: Path,
                                cluster_data: dict,
                                undefineds: List[SymbolRef],
                                focuses: Sequence[str]) -> Tuple[List[Path], List[Path], List[str]]:
    source_map = cluster_object_map(cluster_data)
    basename_map = cluster_object_basename_map(cluster_data)
    notes: List[str] = []
    build_dir_text = cluster_data.get("build_dir", "")
    build_dir = Path(build_dir_text) if build_dir_text else None

    if build_dir is not None:
        notes.append(f"build_dir: {build_dir}")
        notes.append(f"build_dir_exists: {'yes' if build_dir.exists() else 'no'}")

    focused = [item for item in undefineds if symbol_matches_focus(item.raw, item.demangled, focuses)]
    if not focused:
        notes.append("cluster probe found no focused unresolved symbols")
        return [], [], notes

    consumer_objects: List[Path] = []
    for item in focused:
        for referencer in item.referencers:
            path = basename_map.get(referencer)
            if path and path.exists() and path not in consumer_objects:
                consumer_objects.append(path)

    if consumer_objects:
        notes.append(f"reused {len(consumer_objects)} referencer object(s) from cluster build")
    else:
        notes.append("no live referencer objects were available from the cluster build")

    provider_objects: List[Path] = []
    for item in focused:
        for candidate in find_repo_source_candidates(repo_root, item.demangled):
            path = source_map.get(candidate)
            if path and path.exists() and path not in provider_objects:
                provider_objects.append(path)

    if provider_objects:
        notes.append(f"reused {len(provider_objects)} likely provider object(s) from cluster build")
    else:
        notes.append("no live provider objects were inferred from the cluster build")

    output_prefix = build_dir_text[:-6] if build_dir_text.endswith(".build") else cluster_data.get("build_dir", "")
    rerun = [
        "python3", "scripts/report_bootstrap_frontier.py",
        "--output-prefix", output_prefix or "/tmp/bootstrap-selfhost-frontier",
        "--compiler", cluster_data.get("compiler", "./dev/cppgm++"),
        "--host-cxx", cluster_data.get("host_cxx", "/usr/local/opt/llvm/bin/clang++"),
        "--jobs", str(cluster_data.get("jobs", 1)),
        "--frontend", cluster_data.get("frontend", "dev/cppgm++.cpp"),
    ]
    notes.append("rerun: " + " ".join(rerun))

    return consumer_objects, provider_objects, notes


def temporary_object_path(repo_root: Path, src: Path, out_dir: Path) -> Path:
    try:
        relative = src.resolve().relative_to(repo_root)
        base = str(relative)
    except Exception:
        base = str(src.resolve())
    digest = hashlib.sha1(base.encode("utf-8")).hexdigest()[:12]
    out = out_dir / f"{src.stem}.{digest}.o"
    out.parent.mkdir(parents=True, exist_ok=True)
    return out


def stable_source_artifact_stem(repo_root: Path, src: Path) -> str:
    try:
        relative = src.resolve().relative_to(repo_root)
        base = str(relative)
    except Exception:
        base = str(src.resolve())
    digest = hashlib.sha1(base.encode("utf-8")).hexdigest()[:12]
    return f"{src.stem}.{digest}"


def compile_to_object(repo_root: Path,
                      compiler: str,
                      include_dir: str,
                      host_cxx: str,
                      src: Path,
                      out_dir: Path,
                      extra_env: Optional[Dict[str, str]] = None) -> Tuple[Path, subprocess.CompletedProcess]:
    out = temporary_object_path(repo_root, src, out_dir)
    env = os.environ.copy()
    env["CPPGM_HOST_CXX"] = host_cxx
    if extra_env:
        env.update(extra_env)
    proc = run(
        [str((repo_root / compiler).resolve()),
         "-c",
         "-I",
         str((repo_root / include_dir).resolve()),
         "-o",
         str(out),
         str(src.resolve())],
        repo_root,
        env,
    )
    return out, proc


def emit_text_output(repo_root: Path,
                     compiler: str,
                     host_cxx: str,
                     src: Path,
                     emit_flag: str,
                     out_path: Path) -> subprocess.CompletedProcess:
    env = os.environ.copy()
    env["CPPGM_HOST_CXX"] = host_cxx
    return run(
        [str((repo_root / compiler).resolve()),
         emit_flag,
         "-o",
         str(out_path),
         str(src.resolve())],
        repo_root,
        env,
    )


def compile_to_object_stream_trace(repo_root: Path,
                                   compiler: str,
                                   include_dir: str,
                                   host_cxx: str,
                                   src: Path,
                                   out_dir: Path,
                                   stderr_path: Path,
                                   stdout_path: Path,
                                   status_path: Optional[Path],
                                   extra_env: Optional[Dict[str, str]] = None
                                   ) -> Tuple[Path, int, Path, Path, List["TraceEvent"], int]:
    out = temporary_object_path(repo_root, src, out_dir)
    env = os.environ.copy()
    env["CPPGM_HOST_CXX"] = host_cxx
    if extra_env:
        env.update(extra_env)

    cmd = [
        str((repo_root / compiler).resolve()),
        "-c",
        "-I",
        str((repo_root / include_dir).resolve()),
        "-o",
        str(out),
        str(src.resolve()),
    ]

    events: List[TraceEvent] = []
    stderr_lines = 0
    write_status(status_path,
                 phase="compiling",
                 source=short_path(repo_root, str(src)),
                 stderr_lines=0,
                 event_count=0)
    with stdout_path.open("w") as stdout_fh, stderr_path.open("w") as stderr_fh:
        proc = subprocess.Popen(
            cmd,
            cwd=str(repo_root),
            env=env,
            stdout=stdout_fh,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1,
        )
        assert proc.stderr is not None
        for line in proc.stderr:
            stderr_fh.write(line)
            stderr_lines += 1
            event = parse_trace_line(line)
            if event is not None:
                events.append(event)
            if stderr_lines == 1 or stderr_lines % 1000 == 0:
                write_status(status_path,
                             phase="compiling",
                             source=short_path(repo_root, str(src)),
                             stderr_lines=stderr_lines,
                             event_count=len(events))
        proc.stderr.close()
        returncode = proc.wait()

    write_status(status_path,
                 phase="parsing",
                 source=short_path(repo_root, str(src)),
                 stderr_lines=stderr_lines,
                 event_count=len(events),
                 returncode=returncode)
    return out, returncode, stdout_path, stderr_path, events, stderr_lines


def resolve_object_inputs(repo_root: Path,
                          src_texts: Sequence[str],
                          explicit_objects: Sequence[str],
                          cluster_map: Optional[Dict[str, Path]]) -> Tuple[List[Path], List[str]]:
    objects: List[Path] = []
    remaining_sources: List[str] = []

    for obj_text in explicit_objects:
        path = (repo_root / obj_text).resolve() if not os.path.isabs(obj_text) else Path(obj_text)
        if not path.exists():
            raise SystemExit(f"object file not found: {path}")
        objects.append(path)

    for src_text in src_texts:
        resolved_key = src_text
        if os.path.isabs(src_text):
            try:
                resolved_key = str(Path(src_text).resolve().relative_to(repo_root))
            except ValueError:
                resolved_key = src_text
        if cluster_map and resolved_key in cluster_map and cluster_map[resolved_key].exists():
            objects.append(cluster_map[resolved_key])
            continue
        remaining_sources.append(src_text)

    return objects, remaining_sources


def collect_symbols(nm: str, defined_only: bool, files: Sequence[Path]) -> List[str]:
    if not files:
        return []
    str_files = [str(path) for path in files]
    candidates = []
    base = os.path.basename(nm)
    if defined_only:
        candidates.append([nm, "-j", "--defined-only", *str_files])
        if base == "nm":
            candidates.append([nm, "-j", "-U", *str_files])
    else:
        candidates.append([nm, "-j", "-u", *str_files])
        if base == "nm":
            candidates.append([nm, "-u", "-j", *str_files])
    for cmd in candidates:
        proc = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        if proc.returncode == 0:
            return sorted({line.strip() for line in proc.stdout.splitlines() if line.strip()})
    raise SystemExit(f"{nm} failed while collecting {'defined' if defined_only else 'undefined'} symbols")


def split_signature(demangled: str) -> Tuple[str, str]:
    if "(" not in demangled:
        return demangled, ""
    base, rest = demangled.split("(", 1)
    return base.strip(), rest.rsplit(")", 1)[0]


def first_difference(a: str, b: str, radius: int = 50) -> str:
    max_common = 0
    while max_common < len(a) and max_common < len(b) and a[max_common] == b[max_common]:
        max_common += 1
    if max_common == len(a) and max_common == len(b):
        return "no visible difference"
    start = max(0, max_common - radius)
    end_a = min(len(a), max_common + radius)
    end_b = min(len(b), max_common + radius)
    return (
        f"consumer ...{a[start:end_a]}...\n"
        f"provider ...{b[start:end_b]}..."
    )


def diagnose_signature_drift(consumer_demangled: str, provider_demangled: str) -> str:
    suspect_patterns = (
        "&::",
        "allocator<std::__1::allocator",
        "vector<unsigned long::pair",
        "unsigned long&::pair",
    )
    if any(pattern in consumer_demangled for pattern in suspect_patterns):
        return "likely consumer-side type reconstruction or mangling corruption"
    if any(pattern in provider_demangled for pattern in suspect_patterns):
        return "likely provider-side mangling corruption"
    return "same logical function name, but different signature spelling"


def render_provider_diff(data: dict, verbose: bool) -> str:
    lines = []
    lines.append("Provider Diff")
    consumers = data.get("consumers", [])
    providers = data.get("providers", [])
    if consumers:
        shown = ", ".join(consumers[:4])
        suffix = "" if len(consumers) <= 4 else f" (+{len(consumers) - 4} more)"
        lines.append(f"- consumers: {shown}{suffix}")
    if providers:
        shown = ", ".join(providers[:4])
        suffix = "" if len(providers) <= 4 else f" (+{len(providers) - 4} more)"
        lines.append(f"- providers: {shown}{suffix}")
    if data.get("temp_dir"):
        lines.append(f"- temp_dir: {data['temp_dir']}")
    lines.append("")

    lines.append(f"focused_unresolved_symbols: {data.get('focused_unresolved_count', 0)}")
    matches = data.get("matches", [])
    if not matches:
        lines.append("no matching unresolved symbols found")
        return "\n".join(lines)

    for match in matches:
        lines.append("")
        lines.append(f"Focus: {match['consumer_demangled']}")
        if verbose and match.get("consumer_raw"):
            lines.append(f"raw: {match['consumer_raw']}")
        lines.append(f"status: {match['status']}")
        if match.get("provider_demangled"):
            lines.append(f"provider_demangled: {match['provider_demangled']}")
        if match.get("diagnosis"):
            lines.append(f"diagnosis: {match['diagnosis']}")
        if match.get("likely_issue"):
            lines.append(f"likely_issue: {match['likely_issue']}")
        if verbose and match.get("provider_raw"):
            lines.append(f"provider_raw: {match['provider_raw']}")
        if verbose and match.get("first_difference"):
            lines.append("first_difference:")
            lines.append(match["first_difference"])
        elif match.get("drift_excerpt"):
            lines.append("drift_excerpt:")
            lines.append(match["drift_excerpt"])
    return "\n".join(lines)


def build_provider_diff_data(repo_root: Path,
                             compiler: str,
                             include_dir: str,
                             host_cxx: str,
                             nm: str,
                             cxxfilt: str,
                             consumer_sources: Sequence[str],
                             provider_sources: Sequence[str],
                             consumer_objects_in: Sequence[Path],
                             provider_objects_in: Sequence[Path],
                             focuses: Sequence[str],
                             keep_temp: bool) -> dict:
    temp_dir_obj: Optional[tempfile.TemporaryDirectory] = None
    temp_dir: Optional[Path] = None
    try:
        if keep_temp:
            temp_dir = Path(tempfile.mkdtemp(prefix="bootstrap-trace-report."))
        else:
            temp_dir_obj = tempfile.TemporaryDirectory(prefix="bootstrap-trace-report.")
            temp_dir = Path(temp_dir_obj.name)

        consumer_objects: List[Path] = list(consumer_objects_in)
        provider_objects: List[Path] = list(provider_objects_in)
        compile_reports: List[str] = []

        for path in consumer_objects:
            compile_reports.append(f"consumer-object: {path}")
        for path in provider_objects:
            compile_reports.append(f"provider-object: {path}")

        for src_text in consumer_sources:
            src = (repo_root / src_text).resolve() if not os.path.isabs(src_text) else Path(src_text)
            obj, proc = compile_to_object(repo_root, compiler, include_dir, host_cxx, src, temp_dir)
            if proc.returncode != 0:
                raise SystemExit(f"consumer compile failed for {src}:\n{proc.stdout}{proc.stderr}")
            consumer_objects.append(obj)
            compile_reports.append(f"consumer: {src}")

        for src_text in provider_sources:
            src = (repo_root / src_text).resolve() if not os.path.isabs(src_text) else Path(src_text)
            obj, proc = compile_to_object(repo_root, compiler, include_dir, host_cxx, src, temp_dir)
            if proc.returncode != 0:
                raise SystemExit(f"provider compile failed for {src}:\n{proc.stdout}{proc.stderr}")
            provider_objects.append(obj)
            compile_reports.append(f"provider: {src}")

        undefined_raw = collect_symbols(nm, False, consumer_objects)
        defined_raw = collect_symbols(nm, True, provider_objects)
        all_symbols = undefined_raw + defined_raw
        demangled = demangle_symbols(all_symbols, cxxfilt)

        defined_by_raw = {symbol: demangled.get(symbol, symbol) for symbol in defined_raw}
        providers_by_demangled: Dict[str, List[str]] = {}
        providers_by_base: Dict[str, List[Tuple[str, str]]] = {}
        for raw in defined_raw:
            text = demangled.get(raw, raw)
            providers_by_demangled.setdefault(text, []).append(raw)
            providers_by_base.setdefault(base_name(text), []).append((raw, text))

        data = {
            "consumers": [],
            "providers": [],
            "matches": [],
            "focused_unresolved_count": 0,
        }
        if compile_reports:
            consumers = [line.split(": ", 1)[1] for line in compile_reports if line.startswith("consumer")]
            providers = [line.split(": ", 1)[1] for line in compile_reports if line.startswith("provider")]
            if consumers:
                data["consumers"] = [short_path(repo_root, value) for value in consumers]
            if providers:
                data["providers"] = [short_path(repo_root, value) for value in providers]
        if keep_temp:
            data["temp_dir"] = str(temp_dir)

        filtered = [
            raw for raw in undefined_raw
            if symbol_matches_focus(raw, demangled.get(raw, raw), focuses)
        ]
        data["focused_unresolved_count"] = len(filtered)
        if not filtered:
            return data

        for raw in filtered:
            consumer_demangled = demangled.get(raw, raw)
            entry = {
                "consumer_raw": raw,
                "consumer_demangled": consumer_demangled,
            }

            if raw in defined_by_raw:
                entry["status"] = "exact symbol match found in provider set"
                data["matches"].append(entry)
                continue

            exact_demangled = providers_by_demangled.get(consumer_demangled, [])
            if exact_demangled:
                entry["status"] = "mangled-only drift"
                entry["diagnosis"] = "provider demangles identically, but the raw symbol spelling differs"
                entry["likely_issue"] = "caller/provider mangling disagreement, not semantic signature mismatch"
                entry["provider_raw"] = exact_demangled[0]
                data["matches"].append(entry)
                continue

            base = base_name(consumer_demangled)
            candidates = providers_by_base.get(base, [])
            if not candidates:
                entry["status"] = "no provider with the same logical function name"
                data["matches"].append(entry)
                continue

            raw_candidate, demangled_candidate = candidates[0]
            entry["status"] = "same logical name, different signature"
            entry["provider_demangled"] = demangled_candidate
            _, consumer_sig = split_signature(consumer_demangled)
            _, provider_sig = split_signature(demangled_candidate)
            entry["diagnosis"] = diagnose_signature_drift(consumer_demangled, demangled_candidate)
            entry["provider_raw"] = raw_candidate
            entry["first_difference"] = first_difference(consumer_sig, provider_sig)
            entry["drift_excerpt"] = trim(
                entry["first_difference"].replace("\n", " | "), 220)
            data["matches"].append(entry)
        return data
    finally:
        if not keep_temp and temp_dir_obj is not None:
            temp_dir_obj.cleanup()


TRACE_PRESETS: Dict[str, List[str]] = {
    "template-lifecycle": ["template.resolve", "class.collect", "template.scope", "overload"],
    "output-lifecycle": ["output.class", "output.require", "output.export", "output.audit",
                         "lowir.copy",
                         "class.collect"],
    "linkage": ["symbol.linkage", "output.require", "output.export", "output.audit"],
    "full-link-root-cause": [
        "overload",
        "template.resolve",
        "class.collect",
        "output.class",
        "symbol.linkage",
        "output.require",
        "output.export",
        "output.audit",
        "lowir.copy",
        "template.scope",
    ],
}


def simple_focus_mangle(text: str) -> str:
    out = []
    i = 0
    while i < len(text):
        if i + 1 < len(text) and text[i:i + 2] == "::":
            out.append("__")
            i += 2
            continue
        ch = text[i]
        if ch.isalnum() or ch == "_":
            out.append(ch)
        else:
            out.append("_")
        i += 1
    return "".join(out)


def expanded_focus_terms(focuses: Sequence[str]) -> List[str]:
    terms = set()
    for focus in focuses:
        if not focus:
            continue
        stripped = focus.strip()
        if not stripped:
            continue
        base = stripped.split("(", 1)[0].strip()
        tail = base.split("::")[-1].strip()
        mangled = simple_focus_mangle(base)
        candidates = [stripped, base, mangled]
        if "::" not in base:
            candidates.append(tail)
        for candidate in candidates:
            if candidate:
                terms.add(candidate.lower())
    return sorted(terms)


def line_matches_focus(line: str, focus_terms: Sequence[str]) -> bool:
    if not focus_terms:
        return True
    lowered = line.lower()
    return any(term in lowered for term in focus_terms)


def classify_semantic_line(stripped: str) -> str:
    if stripped.startswith("function-declaration ") or stripped.startswith("function-definition "):
        return "declaration"
    if stripped.startswith("variable "):
        return "declaration"
    if stripped.startswith("callee "):
        return "use"
    if stripped.startswith("id-expression "):
        return "use"
    return "other"


def classify_lowir_line(stripped: str) -> str:
    if stripped.startswith("function @") or stripped.startswith("global @"):
        return "definition"
    if "@" in stripped:
        return "reference"
    return "other"


def select_matching_lines(text: str,
                          focus_terms: Sequence[str],
                          classifier) -> Dict[str, List[dict]]:
    out: Dict[str, List[dict]] = {
        "declaration": [],
        "use": [],
        "definition": [],
        "reference": [],
        "other": [],
    }
    for lineno, line in enumerate(text.splitlines(), 1):
        if not line_matches_focus(line, focus_terms):
            continue
        stripped = line.strip()
        category = classifier(stripped)
        out.setdefault(category, []).append({
            "line": lineno,
            "text": stripped,
        })
    return out


def infer_symbol_pipeline_issue(semantic_hits: Dict[str, List[dict]],
                                lowir_hits: Dict[str, List[dict]]) -> str:
    semantic_decls = len(semantic_hits.get("declaration", []))
    semantic_uses = len(semantic_hits.get("use", []))
    lowir_defs = len(lowir_hits.get("definition", []))
    lowir_refs = len(lowir_hits.get("reference", []))

    if semantic_uses and not semantic_decls and lowir_refs and not lowir_defs:
        return ("Likely issue\n"
                "- focused symbol is used in semantic output and referenced in LowIR, "
                "but no semantic declaration/definition node was emitted")
    if semantic_decls and lowir_refs and not lowir_defs:
        return ("Likely issue\n"
                "- focused symbol has semantic declaration/definition output, "
                "but no corresponding LowIR definition/export was emitted")
    if not semantic_decls and not semantic_uses and lowir_refs:
        return ("Likely issue\n"
                "- focused symbol only appears as a LowIR reference, "
                "which suggests the semantic binding never surfaced into output")
    return ""


def symbol_pipeline_data(source_label: str,
                         semantics_text: str,
                         lowir_text: str,
                         focuses: Sequence[str]) -> dict:
    focus_terms = expanded_focus_terms(focuses)
    semantic_hits = select_matching_lines(semantics_text, focus_terms, classify_semantic_line)
    lowir_hits = select_matching_lines(lowir_text, focus_terms, classify_lowir_line)
    return {
        "source": source_label,
        "focus": list(focuses),
        "focus_terms": focus_terms,
        "semantic_hits": semantic_hits,
        "lowir_hits": lowir_hits,
        "likely_issue": infer_symbol_pipeline_issue(semantic_hits, lowir_hits),
    }


def render_symbol_pipeline(data: dict) -> str:
    lines = []
    lines.append("Symbol Pipeline")
    lines.append(f"- source: {data['source']}")
    if data.get("focus"):
        lines.append(f"- focus: {', '.join(data['focus'])}")
    if data.get("focus_terms"):
        lines.append(f"- focus_terms: {', '.join(data['focus_terms'])}")

    semantic_hits = data["semantic_hits"]
    lowir_hits = data["lowir_hits"]
    lines.append("")
    lines.append("Semantic Output")
    lines.append(f"- declarations: {len(semantic_hits.get('declaration', []))}")
    lines.append(f"- uses: {len(semantic_hits.get('use', []))}")
    for entry in semantic_hits.get("declaration", [])[:4]:
        lines.append(f"- decl L{entry['line']}: {trim(entry['text'], 180)}")
    for entry in semantic_hits.get("use", [])[:6]:
        lines.append(f"- use L{entry['line']}: {trim(entry['text'], 180)}")

    lines.append("")
    lines.append("LowIR")
    lines.append(f"- definitions: {len(lowir_hits.get('definition', []))}")
    lines.append(f"- references: {len(lowir_hits.get('reference', []))}")
    for entry in lowir_hits.get("definition", [])[:4]:
        lines.append(f"- def L{entry['line']}: {trim(entry['text'], 180)}")
    for entry in lowir_hits.get("reference", [])[:6]:
        lines.append(f"- ref L{entry['line']}: {trim(entry['text'], 180)}")

    if data.get("likely_issue"):
        lines.append("")
        lines.append(data["likely_issue"])
    return "\n".join(lines)


def run_symbol_pipeline(repo_root: Path,
                        compiler: str,
                        host_cxx: str,
                        source_text: str,
                        focuses: Sequence[str],
                        write_prefix: str,
                        keep_temp: bool) -> Tuple[str, str, dict]:
    temp_dir_obj: Optional[tempfile.TemporaryDirectory] = None
    temp_dir: Optional[Path] = None
    try:
        if keep_temp:
            temp_dir = Path(tempfile.mkdtemp(prefix="bootstrap-trace-report."))
        else:
            temp_dir_obj = tempfile.TemporaryDirectory(prefix="bootstrap-trace-report.")
            temp_dir = Path(temp_dir_obj.name)

        source = (repo_root / source_text).resolve() if not os.path.isabs(source_text) else Path(source_text)
        stem = stable_source_artifact_stem(repo_root, source)
        if write_prefix:
            prefix = Path(write_prefix)
            semantics_path = Path(str(prefix) + f".{stem}.semantics.txt")
            lowir_path = Path(str(prefix) + f".{stem}.lowir.txt")
            semantics_stderr = Path(str(prefix) + f".{stem}.semantics.stderr.txt")
            lowir_stderr = Path(str(prefix) + f".{stem}.lowir.stderr.txt")
        else:
            semantics_path = temp_dir / f"{stem}.semantics.txt"
            lowir_path = temp_dir / f"{stem}.lowir.txt"
            semantics_stderr = temp_dir / f"{stem}.semantics.stderr.txt"
            lowir_stderr = temp_dir / f"{stem}.lowir.stderr.txt"

        semantics_proc = emit_text_output(repo_root,
                                          compiler,
                                          host_cxx,
                                          source,
                                          "--emit-semantics",
                                          semantics_path)
        semantics_stderr.write_text(semantics_proc.stderr)
        if semantics_proc.returncode != 0:
            raise SystemExit(f"semantic emit failed for {source}:\n{semantics_proc.stdout}{semantics_proc.stderr}")

        lowir_proc = emit_text_output(repo_root,
                                      compiler,
                                      host_cxx,
                                      source,
                                      "--emit-lowir",
                                      lowir_path)
        lowir_stderr.write_text(lowir_proc.stderr)
        if lowir_proc.returncode != 0:
            raise SystemExit(f"LowIR emit failed for {source}:\n{lowir_proc.stdout}{lowir_proc.stderr}")

        data = symbol_pipeline_data(short_path(repo_root, str(source)),
                                    semantics_path.read_text(),
                                    lowir_path.read_text(),
                                    focuses)
        data["semantics_path"] = str(semantics_path)
        data["lowir_path"] = str(lowir_path)
        data["semantics_stderr_path"] = str(semantics_stderr)
        data["lowir_stderr_path"] = str(lowir_stderr)

        summary = render_symbol_pipeline(data)
        details = "\n".join([
            "Symbol Pipeline",
            f"source: {source}",
            f"semantics_file: {semantics_path}",
            f"lowir_file: {lowir_path}",
            f"semantics_stderr: {semantics_stderr}",
            f"lowir_stderr: {lowir_stderr}",
            "",
            summary,
        ])
        return summary, details, data
    finally:
        if not keep_temp and temp_dir_obj is not None:
            temp_dir_obj.cleanup()


class TraceEvent:
    __slots__ = ("category", "location", "message", "fields")

    def __init__(self, category: str, location: str, message: str):
        self.category = category
        self.location = location
        self.message = message
        self.fields = parse_kv_fields(message)


def parse_kv_fields(message: str) -> Dict[str, str]:
    matches = list(re.finditer(r'(^| )([A-Za-z0-9_.-]+)=', message))
    if not matches:
        return {}
    fields: Dict[str, str] = {}
    for i, match in enumerate(matches):
        key = match.group(2)
        start = match.end()
        end = matches[i + 1].start() if i + 1 < len(matches) else len(message)
        value = message[start:end].strip()
        fields[key] = value.strip('"')
    return fields


def parse_trace_line(line: str) -> Optional[TraceEvent]:
    line = line.rstrip()
    if line.startswith("ERROR:"):
        return TraceEvent("error", "", "action=compiler-error text=" + line[6:].strip())
    if not line.startswith("["):
        return None
    close = line.find("]")
    if close <= 1:
        return None
    category = line[1:close]
    rest = line[close + 1:].strip()
    if not rest:
        return TraceEvent(category, "", "")

    first, sep, remaining = rest.partition(" ")
    if sep and "=" not in first and (":" in first or "/" in first):
        return TraceEvent(category, first, remaining)
    return TraceEvent(category, "", rest)


def parse_trace_events(stderr_text: str) -> List[TraceEvent]:
    events: List[TraceEvent] = []
    for line in stderr_text.splitlines():
        event = parse_trace_line(line)
        if event is not None:
            events.append(event)
    return events


def event_matches_focus(event: TraceEvent, focuses: Sequence[str]) -> bool:
    if not focuses:
        return True
    haystacks = [event.message, event.location]
    haystacks.extend(event.fields.values())
    lowered = [text.lower() for text in haystacks if text]
    for focus in focuses:
        needle = focus.lower()
        if any(needle in text for text in lowered):
            return True
    return False


def text_matches_focus(text: str, focuses: Sequence[str]) -> bool:
    lowered = text.lower()
    for focus in focuses:
        if focus.lower() in lowered:
            return True
    return False


def focus_entity_values(event: TraceEvent) -> List[str]:
    values: List[str] = []
    for key in ("symbol", "internal", "entity", "owner", "display", "function"):
        value = event.fields.get(key, "")
        if value and value not in values:
            values.append(value)
    return values


def format_trace_event(event: TraceEvent) -> str:
    if event.location:
        return f"[{event.category}] {event.location} {event.message}"
    return f"[{event.category}] {event.message}"


def format_focus_lifecycle_event(event: TraceEvent) -> str:
    if event.category == "symbol.linkage":
        snippet = (f"symbol.linkage {event.fields.get('action', '<unknown>')} "
                   f"entity={event.fields.get('entity', '<unknown>')}")
        if event.fields.get("internal"):
            snippet += f" internal={event.fields['internal']}"
        if event.fields.get("object"):
            snippet += f" object={event.fields['object']}"
        if event.fields.get("linkage"):
            snippet += f" linkage={event.fields['linkage']}"
        return snippet
    if event.category == "output.require":
        snippet = (f"output.require {event.fields.get('action', '<unknown>')} "
                   f"entity={event.fields.get('entity', '<unknown>')}")
        if event.fields.get("reason"):
            snippet += f" reason={event.fields['reason']}"
        if event.fields.get("symbol"):
            snippet += f" symbol={event.fields['symbol']}"
        return snippet
    if event.category == "output.export":
        snippet = (f"output.export {event.fields.get('action', '<unknown>')} "
                   f"symbol={event.fields.get('symbol', '<unknown>')}")
        if event.fields.get("reason"):
            snippet += f" reason={event.fields['reason']}"
        if event.fields.get("owner"):
            snippet += f" owner={event.fields['owner']}"
        if event.fields.get("linkage"):
            snippet += f" linkage={event.fields['linkage']}"
        return snippet
    if event.category == "output.audit":
        snippet = (f"output.audit {event.fields.get('action', '<unknown>')} "
                   f"function={event.fields.get('function', event.fields.get('symbol', '<unknown>'))}")
        if event.fields.get("reason"):
            snippet += f" reason={event.fields['reason']}"
        if event.fields.get("detail"):
            snippet += f" detail={trim(event.fields['detail'], 180)}"
        return snippet
    if event.category == "template.resolve" and event.fields.get("action") == "deduction-type":
        snippet = f"template.resolve deduction-type route={event.fields.get('route', '<unknown>')}"
        if event.fields.get("result"):
            snippet += f" result={event.fields['result']}"
        if event.fields.get("type"):
            snippet += f" type={event.fields['type']}"
        return snippet
    if event.category == "overload" and event.fields.get("action") == "arg-analysis":
        snippet = f"overload arg-analysis mode={event.fields.get('mode', '<unknown>')}"
        if event.fields.get("reason"):
            snippet += f" reason={event.fields['reason']}"
        if event.fields.get("target"):
            snippet += f" target={event.fields['target']}"
        return snippet
    if event.category == "error":
        return "error " + event.fields.get("text", event.message)
    return format_trace_event(event)


def select_focus_entities(events: Sequence[TraceEvent],
                          focuses: Sequence[str],
                          limit: int = 4) -> List[str]:
    if not focuses:
        return []
    counts: Dict[str, int] = {}
    for event in events:
        for value in focus_entity_values(event):
            if text_matches_focus(value, focuses):
                counts[value] = counts.get(value, 0) + 1
    ranked = sorted(counts.items(), key=lambda item: (-item[1], item[0]))
    return [value for value, _ in ranked[:limit]]


def summarize_template_scope(events: Sequence[TraceEvent]) -> List[str]:
    if not events:
        return []
    action_counts: Dict[str, int] = {}
    bindings: List[str] = []
    for event in events:
        action = event.fields.get("action", "<unknown>")
        action_counts[action] = action_counts.get(action, 0) + 1
        binding = event.fields.get("binding", "")
        if binding and binding not in bindings:
            bindings.append(binding)

    lines = ["Template Scope"]
    for action, count in sorted(action_counts.items()):
        lines.append(f"- {action}: {count}")
    if bindings:
        shown = ", ".join(bindings[:6])
        suffix = "" if len(bindings) <= 6 else f" (+{len(bindings) - 6} more)"
        lines.append(f"- bindings: {shown}{suffix}")
    return lines


def infer_trace_issue(symbol_events: Sequence[TraceEvent],
                      require_events: Sequence[TraceEvent],
                      export_events: Sequence[TraceEvent],
                      audit_events: Sequence[TraceEvent],
                      error_events: Sequence[TraceEvent]) -> str:
    for event in audit_events:
        if event.fields.get("action") == "skip-required-definition-validation":
            reason = event.fields.get("reason", "unknown")
            detail = event.fields.get("detail", "")
            return ("Likely issue\n"
                    f"- semantic output skipped required-definition validation because {reason}"
                    + (f" ({trim(detail, 220)})" if detail else ""))
        if event.fields.get("action") == "required-definition-not-emitted":
            detail = event.fields.get("detail", "")
            return ("Likely issue\n"
                    "- semantic output required a definition but never emitted it"
                    + (f" ({trim(detail, 220)})" if detail else ""))
        if event.fields.get("action") == "lowir-missing-semantic-owner":
            detail = event.fields.get("detail", "")
            return ("Likely issue\n"
                    "- LowIR closure found an exported symbol with no semantic output owner"
                    + (f" ({trim(detail, 220)})" if detail else ""))

    insert_reasons_by_symbol: Dict[str, List[str]] = {}
    for event in export_events:
        symbol = event.fields.get("symbol", "")
        if not symbol:
            continue
        if event.fields.get("action") in ("insert", "update"):
            insert_reasons_by_symbol.setdefault(symbol, []).append(
                event.fields.get("reason", "<unknown>"))
    for event in export_events:
        if event.fields.get("action") == "missing-closure":
            symbol = event.fields.get("symbol", "<unknown>")
            if event.fields.get("reason") == "exported-symbol":
                insert_reasons = insert_reasons_by_symbol.get(symbol, [])
                if any(reason in ("callee", "function-id", "id-expression-global")
                       for reason in insert_reasons):
                    return ("Likely issue\n"
                            f"- {symbol} entered exported-symbol closure through "
                            f"{', '.join(insert_reasons)} but never became a semantic owner")
                if (event.fields.get("known-function") == "no" and
                        event.fields.get("known-global") == "no" and
                        event.fields.get("external-function") == "no" and
                        event.fields.get("external-object") == "no" and
                        event.fields.get("referenced-function") == "no" and
                        event.fields.get("referenced-global") == "no"):
                    return ("Likely issue\n"
                            f"- backend closure validation found no semantic, external, "
                            f"or reference owner for {symbol}")
            return ("Likely issue\n"
                    f"- backend closure validation failed for {symbol}")
    for event in export_events:
        if event.fields.get("action") == "prune":
            return ("Likely issue\n"
                    f"- exported symbol was pruned as {event.fields.get('reason', 'unknown')}")
    if any(event.fields.get("action") == "suppress-definition" for event in require_events):
        return ("Likely issue\n"
                "- definition request hit explicit-instantiation suppression")

    inserted_export = any(event.fields.get("action") == "insert" for event in export_events)
    required_definition = any(
        event.fields.get("action") in ("require-definition", "insert-required-definition")
        for event in require_events
    )
    if required_definition and not inserted_export:
        return ("Likely issue\n"
                "- definition was required, but no export/retention event was observed")

    for event in symbol_events:
        if event.fields.get("object", "") == "" and event.fields.get("c-linkage") != "yes":
            return ("Likely issue\n"
                    f"- linkage step produced no object symbol for {event.fields.get('entity', '<unknown>')}")
    for event in error_events:
        text = event.fields.get("text", event.message)
        if "missing semantic owner" in text:
            return ("Likely issue\n"
                    f"- compile failed late in LowIR closure validation: {text}")
    return ""


def trace_summary_data(source_label: str,
                       preset: str,
                       events: Sequence[TraceEvent],
                       focuses: Sequence[str]) -> dict:
    focused_events = [event for event in events if event_matches_focus(event, focuses)]
    selected = focused_events if focused_events else list(events)

    counts: Dict[str, int] = {}
    for event in selected:
        counts[event.category] = counts.get(event.category, 0) + 1

    symbol_events = [event for event in selected if event.category == "symbol.linkage"]
    require_events = [event for event in selected if event.category == "output.require"]
    export_events = [event for event in selected if event.category == "output.export"]
    audit_events = [event for event in selected if event.category == "output.audit"]
    scope_events = [event for event in selected if event.category == "template.scope"]
    error_events = [event for event in selected if event.category == "error"]
    legacy_events = [event for event in selected
                     if event.category in ("template.resolve", "class.collect",
                                           "output.class", "overload")]

    focus_entities = select_focus_entities(selected, focuses)
    focus_lifecycle = []
    for entity in focus_entities:
        entity_events = [event for event in selected if entity in focus_entity_values(event)]
        focus_lifecycle.append({
            "entity": entity,
            "events": [format_focus_lifecycle_event(event) for event in entity_events[:8]],
        })

    lifecycle = []
    for group in (symbol_events[:4], require_events[:4], export_events[:4], audit_events[:4]):
        for event in group:
            lifecycle.append({
                "category": event.category,
                "action": event.fields.get("action", "<unknown>"),
                "entity": event.fields.get("entity",
                                           event.fields.get("function",
                                                            event.fields.get("symbol",
                                                                             "<unknown>"))),
                "reason": event.fields.get("reason", ""),
                "object": event.fields.get("object", ""),
            })

    template_scope = {
        "actions": {},
        "bindings": [],
    }
    action_counts: Dict[str, int] = {}
    bindings: List[str] = []
    for event in scope_events:
        action = event.fields.get("action", "<unknown>")
        action_counts[action] = action_counts.get(action, 0) + 1
        binding = event.fields.get("binding", "")
        if binding and binding not in bindings:
            bindings.append(binding)
    template_scope["actions"] = dict(sorted(action_counts.items()))
    template_scope["bindings"] = bindings

    return {
        "source": source_label,
        "preset": preset,
        "trace_event_count": len(events),
        "focused_event_count": len(selected),
        "category_counts": dict(sorted(counts.items())),
        "lifecycle": lifecycle,
        "focus_lifecycle": focus_lifecycle,
        "compiler_errors": [event.fields.get("text", event.message) for event in error_events[:4]],
        "template_scope": template_scope,
        "audit_trace": [format_focus_lifecycle_event(event) for event in audit_events[:6]],
        "supporting_trace": [format_trace_event(event) for event in legacy_events[:6]],
        "likely_issue": infer_trace_issue(symbol_events,
                                          require_events,
                                          export_events,
                                          audit_events,
                                          error_events),
    }


def summarize_trace_events(source_label: str,
                           preset: str,
                           events: Sequence[TraceEvent],
                           focuses: Sequence[str]) -> str:
    data = trace_summary_data(source_label, preset, events, focuses)

    lines = []
    lines.append("Focused Trace")
    lines.append(f"- source: {data['source']}")
    lines.append(f"- preset: {data['preset']}")
    lines.append(f"- trace-events: {data['trace_event_count']}")
    lines.append(f"- focused-events: {data['focused_event_count']}")
    if data["category_counts"]:
        lines.append("- categories: " + ", ".join(
            f"{category}={count}" for category, count in sorted(data["category_counts"].items())))

    if data["lifecycle"]:
        lines.append("")
        lines.append("Lifecycle")
        for event in data["lifecycle"]:
            snippet = f"- {event['category']} {event['action']} {event['entity']}"
            if event.get("reason"):
                snippet += f" [{event['reason']}]"
            if event.get("object"):
                snippet += f" object={event['object']}"
            lines.append(snippet)

    if data["focus_lifecycle"]:
        lines.append("")
        lines.append("Focused Entities")
        for entry in data["focus_lifecycle"]:
            lines.append(f"- {entry['entity']}")
            for event_text in entry["events"][:5]:
                lines.append(f"  {event_text}")

    if data["template_scope"]["actions"]:
        lines.append("")
        lines.append("Template Scope")
        for action, count in data["template_scope"]["actions"].items():
            lines.append(f"- {action}: {count}")
        if data["template_scope"]["bindings"]:
            shown = ", ".join(data["template_scope"]["bindings"][:6])
            suffix = "" if len(data["template_scope"]["bindings"]) <= 6 else f" (+{len(data['template_scope']['bindings']) - 6} more)"
            lines.append(f"- bindings: {shown}{suffix}")

    if data["audit_trace"]:
        lines.append("")
        lines.append("Audit Trace")
        for event in data["audit_trace"][:6]:
            lines.append(f"- {event}")

    if data["supporting_trace"]:
        lines.append("")
        lines.append("Supporting Trace")
        for event in data["supporting_trace"][:6]:
            lines.append(f"- {event}")

    if data["compiler_errors"]:
        lines.append("")
        lines.append("Compiler Errors")
        for error_text in data["compiler_errors"]:
            lines.append(f"- {error_text}")

    if data["likely_issue"]:
        lines.append("")
        lines.append(data["likely_issue"])

    return "\n".join(lines)


def summarize_raw_trace(source_label: str,
                        preset: str,
                        stderr_text: str,
                        focuses: Sequence[str]) -> Tuple[str, str, dict]:
    events = parse_trace_events(stderr_text)
    data = trace_summary_data(source_label, preset, events, focuses)
    summary = summarize_trace_events(source_label, preset, events, focuses)

    details: List[str] = []
    details.append("Raw Trace Analysis")
    details.append(f"source: {source_label}")
    details.append(f"preset: {preset}")
    details.append(f"stderr_lines: {len(stderr_text.splitlines())}")
    details.append(f"trace_events: {len(events)}")
    details.append("")
    details.append(summary)
    raw_lines = stderr_text.splitlines()
    if raw_lines:
        details.append("")
        details.append("Raw Trace Excerpt")
        excerpt = raw_lines[:200]
        details.append("\n".join(excerpt))
        if len(raw_lines) > len(excerpt):
            details.append(f"... ({len(raw_lines) - len(excerpt)} more lines in raw trace)")

    data["stderr_line_count"] = len(raw_lines)
    data["exit_status"] = None
    data["raw_trace_source"] = source_label
    return summary, "\n".join(details), data


def run_focused_trace(repo_root: Path,
                      compiler: str,
                      include_dir: str,
                      host_cxx: str,
                      source_text: str,
                      preset: str,
                      focuses: Sequence[str],
                      trace_file_filter: str,
                      trace_limit: int,
                      write_prefix: str,
                      keep_temp: bool) -> Tuple[str, str, dict]:
    temp_dir_obj: Optional[tempfile.TemporaryDirectory] = None
    temp_dir: Optional[Path] = None
    try:
        if keep_temp:
            temp_dir = Path(tempfile.mkdtemp(prefix="bootstrap-trace-report."))
        else:
            temp_dir_obj = tempfile.TemporaryDirectory(prefix="bootstrap-trace-report.")
            temp_dir = Path(temp_dir_obj.name)

        source = (repo_root / source_text).resolve() if not os.path.isabs(source_text) else Path(source_text)
        categories = TRACE_PRESETS[preset]
        trace_env = {
            "CPPGM_TRACE": ",".join(categories),
            "CPPGM_TRACE_LIVE": "1",
            "CPPGM_TRACE_LIMIT": str(max(trace_limit, 1)),
            "CPPGM_TRACE_ON_ERROR": "1",
        }
        if trace_file_filter:
            trace_env["CPPGM_TRACE_FILE"] = trace_file_filter
        if len(focuses) == 1:
            trace_env["CPPGM_TRACE_SYMBOL"] = focuses[0]

        persistent_prefix = Path(write_prefix) if write_prefix else None
        if persistent_prefix is not None:
            raw_stdout_path = Path(str(persistent_prefix) + ".stdout.txt")
            raw_stderr_path = Path(str(persistent_prefix) + ".stderr.txt")
            status_path = Path(str(persistent_prefix) + ".status")
        else:
            raw_stdout_path = temp_dir / "trace.stdout.txt"
            raw_stderr_path = temp_dir / "trace.stderr.txt"
            status_path = temp_dir / "trace.status"

        obj, returncode, stdout_path, stderr_path, events, stderr_lines = (
            compile_to_object_stream_trace(repo_root,
                                           compiler,
                                           include_dir,
                                           host_cxx,
                                           source,
                                           temp_dir,
                                           raw_stderr_path,
                                           raw_stdout_path,
                                           status_path,
                                           extra_env=trace_env))
        source_label = short_path(repo_root, str(source))
        write_status(status_path,
                     phase="summarizing",
                     source=source_label,
                     stderr_lines=stderr_lines,
                     event_count=len(events),
                     returncode=returncode)
        data = trace_summary_data(source_label, preset, events, focuses)
        summary = summarize_trace_events(source_label, preset, events, focuses)

        details: List[str] = []
        details.append("Trace Rerun")
        details.append(f"source: {source}")
        details.append(f"preset: {preset}")
        details.append("categories: " + ", ".join(categories))
        details.append("object: " + str(obj))
        details.append(f"exit_status: {returncode}")
        details.append(f"stderr_lines: {stderr_lines}")
        if trace_file_filter:
            details.append(f"file_filter: {trace_file_filter}")
        if focuses:
            details.append("focus: " + ", ".join(focuses))
        details.append(f"raw_trace_file: {stderr_path}")
        details.append(f"compiler_stdout_file: {stdout_path}")
        details.append(f"status_file: {status_path}")
        details.append("")
        details.append(summary)
        if not write_prefix and not keep_temp:
            raw_lines = stderr_path.read_text().splitlines()
            if raw_lines:
                details.append("")
                details.append("Raw Trace Excerpt")
                excerpt = raw_lines[:200]
                details.append("\n".join(excerpt))
                if len(raw_lines) > len(excerpt):
                    details.append(f"... ({len(raw_lines) - len(excerpt)} more lines in streamed trace file)")
            stdout_text = stdout_path.read_text().strip()
            if stdout_text:
                details.append("")
                details.append("Compiler Stdout Excerpt")
                details.append("\n".join(stdout_text.splitlines()[:50]))
        data["stderr_path"] = str(stderr_path)
        data["stdout_path"] = str(stdout_path)
        data["status_path"] = str(status_path)
        data["stderr_line_count"] = stderr_lines
        data["exit_status"] = returncode
        write_status(status_path,
                     phase="done",
                     source=source_label,
                     stderr_lines=stderr_lines,
                     event_count=len(events),
                     returncode=returncode)
        return summary, "\n".join(details), data
    finally:
        if not keep_temp and temp_dir_obj is not None:
            temp_dir_obj.cleanup()


def write_detail_report(text: str) -> Path:
    stamp = int(time.time())
    path = Path(tempfile.gettempdir()) / f"bootstrap-trace-report.{stamp}.{os.getpid()}.txt"
    path.write_text(text)
    return path


def write_persistent_outputs(prefix_text: str,
                             concise_text: str,
                             detailed_text: str,
                             json_sections: Sequence[dict],
                             metadata: Optional[dict] = None) -> Dict[str, str]:
    prefix = Path(prefix_text)
    prefix.parent.mkdir(parents=True, exist_ok=True)
    text_path = Path(str(prefix) + ".txt")
    detail_path = Path(str(prefix) + ".details.txt")
    json_path = Path(str(prefix) + ".json")
    text_path.write_text(concise_text + ("\n" if not concise_text.endswith("\n") else ""))
    detail_path.write_text(detailed_text + ("\n" if not detailed_text.endswith("\n") else ""))
    payload = {
        "sections": list(json_sections),
        "detail_path": str(detail_path),
        "text_path": str(text_path),
    }
    if metadata:
        payload["meta"] = metadata
    json_path.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n")
    return {
        "text_path": str(text_path),
        "detail_path": str(detail_path),
        "json_path": str(json_path),
    }


def main():
    parsed = parse_args()
    repo_root = Path(parsed.repo_root).resolve()
    nm = resolve_tool(parsed.nm, "nm")
    cxxfilt = resolve_tool(parsed.cxxfilt, "c++filt")

    concise_outputs: List[str] = []
    detailed_outputs: List[str] = []
    json_sections: List[dict] = []

    cluster_data = None
    cluster_path: Optional[Path] = None
    cluster_source_objects: Optional[Dict[str, Path]] = None
    cluster_basename_objects: Optional[Dict[str, Path]] = None
    frontier_undefineds: Optional[List[SymbolRef]] = None

    if parsed.cluster:
        cluster_path = Path(parsed.cluster).resolve()
        cluster_data, undefineds, duplicates = load_frontier_link_issues(cluster_path)
        frontier_undefineds = undefineds
        cluster_source_objects = cluster_object_map(cluster_data)
        cluster_basename_objects = cluster_object_basename_map(cluster_data)
        demangled_map = demangle_symbols(
            [item.raw for item in undefineds] + [item.raw for item in duplicates],
            cxxfilt)
        for item in undefineds + duplicates:
            item.demangled = demangled_map.get(item.raw, item.raw)
        json_sections.append({
            "kind": "frontier_summary",
            "data": frontier_summary_data(repo_root,
                                          cluster_data,
                                          undefineds,
                                          duplicates,
                                          parsed.focus,
                                          parsed.top,
                                          True),
        })
        concise_outputs.append(render_frontier_summary(repo_root,
                                                       cluster_data,
                                                       undefineds,
                                                       duplicates,
                                                       parsed.focus,
                                                       parsed.top,
                                                       False))
        detailed_outputs.append(render_frontier_summary(repo_root,
                                                        cluster_data,
                                                        undefineds,
                                                        duplicates,
                                                        parsed.focus,
                                                        parsed.top,
                                                        True))
        if cluster_data.get("result") == "link-failed":
            integration = frontier_integration_data(repo_root,
                                                    cluster_path,
                                                    cluster_data,
                                                    undefineds,
                                                    duplicates,
                                                    parsed.focus,
                                                    parsed.top)
            json_sections.append({
                "kind": "frontier_integration",
                "data": integration,
            })
            concise_outputs.append(render_frontier_integration(integration))
            detailed_outputs.append(render_frontier_integration(integration))

    if parsed.link_stdout:
        link_text = read_text(Path(parsed.link_stdout))
        undefineds = parse_linker_undefineds(link_text)
        duplicates = parse_linker_duplicates(link_text)
        demangled_map = demangle_symbols(
            [item.raw for item in undefineds] + [item.raw for item in duplicates],
            cxxfilt)
        for item in undefineds + duplicates:
            item.demangled = demangled_map.get(item.raw, item.raw)
        link_data = {"result": "link-failed", "active_frontier": "link-stdout"}
        json_sections.append({
            "kind": "frontier_summary",
            "data": frontier_summary_data(repo_root,
                                          link_data,
                                          undefineds,
                                          duplicates,
                                          parsed.focus,
                                          parsed.top,
                                          True),
        })
        concise_outputs.append(render_frontier_summary(repo_root,
                                                       link_data,
                                                       undefineds,
                                                       duplicates,
                                                       parsed.focus,
                                                       parsed.top,
                                                       False))
        detailed_outputs.append(render_frontier_summary(repo_root,
                                                        link_data,
                                                        undefineds,
                                                        duplicates,
                                                        parsed.focus,
                                                        parsed.top,
                                                        True))

    wants_diff = (
        parsed.consumer or parsed.provider or
        parsed.consumer_object or parsed.provider_object or
        parsed.cluster_probe
    )
    if wants_diff:
        skip_diff = False
        if parsed.cluster_probe:
            if cluster_data is None:
                raise SystemExit("--cluster-probe requires --cluster")
            if not parsed.focus:
                raise SystemExit("--cluster-probe requires at least one --focus")
            inferred_consumers, inferred_providers, probe_notes = infer_cluster_probe_objects(
                repo_root, cluster_data, frontier_undefineds or [], parsed.focus)
            parsed.consumer_object.extend(str(path) for path in inferred_consumers)
            parsed.provider_object.extend(str(path) for path in inferred_providers)
            missing_consumer = not (parsed.consumer or parsed.consumer_object)
            missing_provider = not (parsed.provider or parsed.provider_object)
            if missing_consumer:
                probe_notes.append("cluster probe did not infer any consumer inputs; skipping provider diff")
            if missing_provider:
                probe_notes.append("cluster probe did not infer any provider inputs; skipping provider diff")
            json_sections.append({
                "kind": "cluster_probe",
                "data": {
                    "notes": probe_notes,
                    "consumer_objects": [str(path) for path in inferred_consumers],
                    "provider_objects": [str(path) for path in inferred_providers],
                },
            })
            concise_outputs.append("Cluster Probe\n" + "\n".join(f"- {note}" for note in probe_notes))
            detailed_outputs.append("Cluster Probe\n" + "\n".join(f"- {note}" for note in probe_notes))
            if missing_consumer or missing_provider:
                skip_diff = True

        if not skip_diff and not (parsed.consumer or parsed.consumer_object):
            raise SystemExit("supply at least one of --consumer, --consumer-object, or --cluster-probe")
        if not skip_diff and not (parsed.provider or parsed.provider_object):
            raise SystemExit("supply at least one of --provider, --provider-object, or --cluster-probe")

        if skip_diff:
            detail_path = write_detail_report("\n\n".join(detailed_outputs) + "\n")
            if parsed.json_output:
                sys.stdout.write(json.dumps({
                    "sections": json_sections,
                    "detail_path": str(detail_path),
                }, indent=2, sort_keys=True) + "\n")
            else:
                out = "\n\n".join(concise_outputs)
                out += f"\n\ndetails: {detail_path}\n"
                sys.stdout.write(out)
            return

        consumer_objects, remaining_consumers = resolve_object_inputs(
            repo_root,
            parsed.consumer,
            parsed.consumer_object,
            cluster_source_objects,
        )
        provider_objects, remaining_providers = resolve_object_inputs(
            repo_root,
            parsed.provider,
            parsed.provider_object,
            cluster_source_objects,
        )

        if (not consumer_objects and not remaining_consumers and cluster_data and
                parsed.focus and cluster_basename_objects):
            for item in frontier_undefineds or []:
                if not symbol_matches_focus(item.raw, item.demangled, parsed.focus):
                    continue
                for referencer in item.referencers:
                    path = cluster_basename_objects.get(referencer)
                    if path and path.exists() and path not in consumer_objects:
                        consumer_objects.append(path)

        provider_diff_data = build_provider_diff_data(
            repo_root=repo_root,
            compiler=parsed.compiler,
            include_dir=parsed.include_dir,
            host_cxx=parsed.host_cxx,
            nm=nm,
            cxxfilt=cxxfilt,
            consumer_sources=remaining_consumers,
            provider_sources=remaining_providers,
            consumer_objects_in=consumer_objects,
            provider_objects_in=provider_objects,
            focuses=parsed.focus,
            keep_temp=parsed.keep_temp,
        )
        json_sections.append({"kind": "provider_diff", "data": provider_diff_data})
        concise_outputs.append(render_provider_diff(provider_diff_data, False))
        detailed_outputs.append(render_provider_diff(provider_diff_data, True))

    if parsed.trace_source:
        if not parsed.preset:
            raise SystemExit("--trace-source requires --preset")
        for source_text in parsed.trace_source:
            summary, details, trace_data = run_focused_trace(
                repo_root=repo_root,
                compiler=parsed.compiler,
                include_dir=parsed.include_dir,
                host_cxx=parsed.host_cxx,
                source_text=source_text,
                preset=parsed.preset,
                focuses=parsed.focus,
                trace_file_filter=parsed.trace_file_filter,
                trace_limit=parsed.trace_limit,
                write_prefix=parsed.write_prefix,
                keep_temp=parsed.keep_temp,
            )
            json_sections.append({"kind": "focused_trace", "data": trace_data})
            concise_outputs.append(summary)
            detailed_outputs.append(details)

    if parsed.trace_stderr:
        preset = parsed.preset or "raw-trace"
        for trace_path_text in parsed.trace_stderr:
            stderr_text, trace_source_label = read_cli_text(trace_path_text)
            summary, details, trace_data = summarize_raw_trace(
                trace_source_label,
                preset,
                stderr_text,
                parsed.focus,
            )
            json_sections.append({"kind": "focused_trace", "data": trace_data})
            concise_outputs.append(summary)
            detailed_outputs.append(details)

    if parsed.pipeline_source:
        if not parsed.focus:
            raise SystemExit("--pipeline-source requires at least one --focus")
        for source_text in parsed.pipeline_source:
            summary, details, pipeline_data = run_symbol_pipeline(
                repo_root=repo_root,
                compiler=parsed.compiler,
                host_cxx=parsed.host_cxx,
                source_text=source_text,
                focuses=parsed.focus,
                write_prefix=parsed.write_prefix,
                keep_temp=parsed.keep_temp,
            )
            json_sections.append({"kind": "symbol_pipeline", "data": pipeline_data})
            concise_outputs.append(summary)
            detailed_outputs.append(details)

    if not concise_outputs:
        raise SystemExit("supply at least one of --cluster/--link-stdout, "
                         "--consumer/--provider, --trace-source, --trace-stderr, "
                         "or --pipeline-source")

    concise_text = "\n\n".join(concise_outputs)
    detailed_text = "\n\n".join(detailed_outputs) + "\n"
    persisted_paths: Dict[str, str] = {}
    write_metadata = {
        "request": {
            "cluster": str(cluster_path) if cluster_path else "",
            "link_stdout": str(Path(parsed.link_stdout).resolve()) if parsed.link_stdout else "",
            "focus": list(parsed.focus),
            "top": parsed.top,
            "trace_source": list(parsed.trace_source),
            "trace_stderr": [
                str(Path(path_text).resolve()) if path_text != "-" else "-"
                for path_text in parsed.trace_stderr
            ],
            "preset": parsed.preset,
            "cluster_probe": bool(parsed.cluster_probe),
            "consumer": list(parsed.consumer),
            "provider": list(parsed.provider),
            "consumer_object": list(parsed.consumer_object),
            "provider_object": list(parsed.provider_object),
        },
        "inputs": {},
        "tool": {
            "path": str(Path(__file__).resolve()),
            "sha256": file_sha256(Path(__file__).resolve()),
        },
    }
    if cluster_path:
        write_metadata["inputs"]["cluster_sha256"] = file_sha256(cluster_path)
    if parsed.link_stdout:
        write_metadata["inputs"]["link_stdout_sha256"] = file_sha256(Path(parsed.link_stdout).resolve())
    if parsed.trace_stderr:
        raw_trace_hashes = {}
        for trace_path_text in parsed.trace_stderr:
            if trace_path_text == "-":
                continue
            raw_trace_hashes[str(Path(trace_path_text).resolve())] = file_sha256(
                Path(trace_path_text).resolve())
        if raw_trace_hashes:
            write_metadata["inputs"]["trace_stderr_sha256"] = raw_trace_hashes
    if parsed.write_prefix:
        persisted_paths = write_persistent_outputs(parsed.write_prefix,
                                                   concise_text,
                                                   detailed_text,
                                                   json_sections,
                                                   write_metadata)
        detail_path = Path(persisted_paths["detail_path"])
    else:
        detail_path = write_detail_report(detailed_text)
    if parsed.json_output:
        payload = {
            "sections": json_sections,
            "detail_path": str(detail_path),
        }
        payload.update(persisted_paths)
        sys.stdout.write(json.dumps(payload, indent=2, sort_keys=True) + "\n")
    else:
        out = concise_text
        if persisted_paths:
            out += "\n\n"
            out += "persisted: "
            out += ", ".join(f"{key}={value}" for key, value in sorted(persisted_paths.items()))
        out += f"\n\ndetails: {detail_path}\n"
        sys.stdout.write(out)


if __name__ == "__main__":
    main()

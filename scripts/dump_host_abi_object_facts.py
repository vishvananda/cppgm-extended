#!/usr/bin/env python3

import argparse
import pathlib
import sys
from typing import List, Sequence

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))

from dump_private_eh_object_facts import (
    detect_format,
    parse_elf_relocations,
    parse_elf_sections,
    parse_macho_relocations,
    parse_macho_sections,
    canonical_symbol,
    run_command,
)


INTERESTING_SECTIONS = {
    "text",
    "data",
    "rodata",
}

RUNTIME_ABI_SYMBOLS = {
    "__dynamic_cast",
    "__cxa_bad_cast",
    "__cxa_bad_typeid",
    "__cxa_pure_virtual",
    "__cxa_deleted_virtual",
}


def is_host_abi_symbol(symbol: str) -> bool:
    return symbol.startswith("_ZT") or symbol in RUNTIME_ABI_SYMBOLS


def parse_demangled_defined_symbols(text: str) -> List[str]:
    symbols = set()
    for raw_line in text.splitlines():
        line = raw_line.strip()
        if not line:
            continue
        parts = line.split(None, 2)
        if len(parts) == 2:
            kind, symbol = parts
        elif len(parts) >= 3:
            _, kind, symbol = parts[:3]
        else:
            continue
        if kind.upper() == "U":
            continue
        symbols.add(symbol.strip())
    return sorted(symbols)


def parse_raw_nm_output(text: str, fmt: str) -> tuple[List[str], List[str]]:
    defined = set()
    undefined = set()
    for raw_line in text.splitlines():
        line = raw_line.strip()
        if not line:
            continue
        parts = line.split()
        if len(parts) == 2:
            kind, symbol = parts
        elif len(parts) >= 3:
            _, kind, symbol = parts[:3]
        else:
            continue
        symbol = canonical_symbol(fmt, symbol)
        upper_kind = kind.upper()
        if upper_kind == "U":
            undefined.add(symbol)
        elif upper_kind in {"T", "D", "B", "S", "R", "V", "W"}:
            defined.add(symbol)
    return sorted(defined), sorted(undefined)


def is_interesting_thunk(symbol: str) -> bool:
    return "thunk to " in symbol


def is_interesting_reloc(section: str, target: str) -> bool:
    return (
        is_host_abi_symbol(target)
        or (section in {"data", "rodata"} and target.startswith("section:text"))
        or (section in {"data", "rodata"} and target.startswith("section:rodata"))
        or (
            section in {"data", "rodata"}
            and (target.startswith("_Z") or target in RUNTIME_ABI_SYMBOLS)
        )
    )


def dump_facts(path: pathlib.Path) -> List[str]:
    fmt = detect_format(path)
    facts = [f"object_format {fmt}"]

    nm_output = run_command(["nm", "-g", str(path)])
    defined, undefined = parse_raw_nm_output(nm_output, fmt)
    for symbol in defined:
        if is_host_abi_symbol(symbol):
            facts.append(f"define {symbol}")
    for symbol in undefined:
        if is_host_abi_symbol(symbol):
            facts.append(f"undef {symbol}")

    demangled_output = run_command(["nm", "-g", "-C", str(path)])
    for symbol in parse_demangled_defined_symbols(demangled_output):
        if is_interesting_thunk(symbol):
            facts.append(f"define_thunk {symbol}")

    if fmt == "mach-o":
        sections = parse_macho_sections(run_command(["/usr/bin/otool", "-l", str(path)]))
        relocs = parse_macho_relocations(run_command(["/usr/bin/otool", "-rv", str(path)]))
    else:
        sections = parse_elf_sections(run_command(["readelf", "-SW", str(path)]))
        relocs = parse_elf_relocations(run_command(["readelf", "-rW", str(path)]))

    for section in sections:
        if section in INTERESTING_SECTIONS:
            facts.append(f"section {section}")

    for section, reloc_type, target in relocs:
        if is_interesting_reloc(section, target):
            facts.append(f"reloc {section} {reloc_type} {target}")

    return facts


def main(argv: Sequence[str]) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("object_file")
    args = parser.parse_args(argv)
    for line in dump_facts(pathlib.Path(args.object_file)):
        print(line)
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))

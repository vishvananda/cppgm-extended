#!/usr/bin/env python3

import argparse
import pathlib
import sys
from typing import List, Sequence

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))

from dump_private_eh_object_facts import (
    canonical_symbol,
    detect_format,
    parse_elf_relocations,
    parse_elf_sections,
    parse_macho_relocations,
    parse_macho_sections,
    parse_nm_output,
    run_command,
)


INTERESTING_SECTIONS = {
    "text",
    "data",
    "rodata",
    "compact_unwind",
    "eh_frame",
    "gcc_except_table",
}


def is_host_eh_symbol(symbol: str) -> bool:
    return (
        symbol.startswith("__cxa_")
        or symbol == "__gxx_personality_v0"
        or symbol == "_Unwind_Resume"
        or symbol.startswith("_ZTI")
        or symbol.startswith("_ZTS")
        or symbol.startswith("_ZTVN10__cxxabiv1")
    )


def is_interesting_reloc(section: str, target: str) -> bool:
    return (
        is_host_eh_symbol(target)
        or target.startswith("section:eh_frame")
        or target.startswith("section:gcc_except_table")
        or target.startswith("section:compact_unwind")
        or target.startswith("section:text")
        or target.startswith("cppgm_lsda_")
        or section in {"compact_unwind", "eh_frame", "gcc_except_table"}
    )


def dump_facts(path: pathlib.Path) -> List[str]:
    fmt = detect_format(path)
    facts = [f"object_format {fmt}"]

    nm_output = run_command(["nm", "-g", str(path)])
    defined, undefined = parse_nm_output(nm_output, fmt)
    for symbol in defined:
        if is_host_eh_symbol(symbol):
            facts.append(f"define {symbol}")
    for symbol in undefined:
        if is_host_eh_symbol(symbol):
            facts.append(f"undef {symbol}")

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

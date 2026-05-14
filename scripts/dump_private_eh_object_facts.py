#!/usr/bin/env python3

import argparse
import pathlib
import re
import subprocess
import sys
from typing import Iterable, List, Sequence, Tuple


PRIVATE_PREFIX = "cppgm_priv_exc_"
INTERESTING_SECTIONS = {
    "text",
    "data",
    "rodata",
    "compact_unwind",
    "eh_frame",
    "gcc_except_table",
}


def run_command(argv: Sequence[str]) -> str:
    result = subprocess.run(
        list(argv),
        text=True,
        capture_output=True,
        check=True,
    )
    return result.stdout


def detect_format(path: pathlib.Path) -> str:
    data = path.read_bytes()[:4]
    if data == b"\x7fELF":
        return "elf"
    if len(data) == 4:
        little = int.from_bytes(data, "little")
        big = int.from_bytes(data, "big")
        if little == 0xFEEDFACF or big == 0xFEEDFACF:
            return "mach-o"
    raise ValueError(f"unsupported object format: {path}")


def canonical_symbol(fmt: str, symbol: str) -> str:
    if fmt == "mach-o" and symbol.startswith("_"):
        return symbol[1:]
    return symbol


def classify_section(name: str) -> str:
    mapping = {
        "__TEXT,__text": "text",
        "__TEXT,__eh_frame": "eh_frame",
        "__TEXT,__gcc_except_tab": "gcc_except_table",
        "__DATA,__data": "data",
        "__DATA,__const": "rodata",
        "__DATA_CONST,__const": "rodata",
        "__LD,__compact_unwind": "compact_unwind",
        ".text": "text",
        ".data": "data",
        ".eh_frame": "eh_frame",
        ".gcc_except_table": "gcc_except_table",
        ".rodata": "rodata",
    }
    if name in mapping:
        return mapping[name]
    if name.startswith(".text."):
        return "text"
    if name.startswith(".data."):
        return "data"
    if name.startswith(".rodata."):
        return "rodata"
    return name


def classify_target(fmt: str, raw_target: str) -> str:
    target = raw_target.strip()
    if fmt == "mach-o":
        section_match = re.search(r"\(([^)]+)\)", target)
        if section_match:
            return f"section:{classify_section(section_match.group(1))}"
    if target.startswith("."):
        return f"section:{classify_section(target)}"
    return canonical_symbol(fmt, target)


def parse_nm_output(text: str, fmt: str) -> Tuple[List[str], List[str]]:
    defined: List[str] = []
    undefined: List[str] = []
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
        if kind.upper() == "U":
            undefined.append(symbol)
        elif kind.upper() in {"T", "D", "B", "S", "R"}:
            defined.append(symbol)
    return sorted(set(defined)), sorted(set(undefined))


def parse_macho_sections(text: str) -> List[str]:
    sections: List[str] = []
    sectname = None
    segname = None
    in_section = False
    for raw_line in text.splitlines():
        line = raw_line.strip()
        if line == "Section":
            in_section = True
            sectname = None
            segname = None
            continue
        if not in_section:
            continue
        if line.startswith("sectname "):
            sectname = line.split(None, 1)[1]
        elif line.startswith("segname "):
            segname = line.split(None, 1)[1]
        elif line.startswith("addr ") or line.startswith("size "):
            if sectname is not None and segname is not None:
                sections.append(classify_section(f"{segname},{sectname}"))
                in_section = False
    return sorted(set(sections))


def normalize_macho_reloc_type(reloc_type: str) -> str:
    value = reloc_type.lower()
    mapping = {
        "signed": "pcrel32",
        "branch": "branch32",
        "unsigned": "abs64",
        "unsignd": "abs64",
        "got_ld": "gotpcrel",
        "got": "got",
    }
    return mapping.get(value, value)


def parse_macho_relocations(text: str) -> List[Tuple[str, str, str]]:
    relocs: List[Tuple[str, str, str]] = []
    current_section = None
    for raw_line in text.splitlines():
        header = re.match(r"^Relocation information \(([^)]+)\)", raw_line.strip())
        if header:
            current_section = classify_section(header.group(1))
            continue
        if current_section is None:
            continue
        line = raw_line.strip()
        if not line or line.startswith("address"):
            continue
        entry = re.match(
            r"^[0-9A-Fa-f]+\s+\S+\s+.+?\s+(?:True|False)\s+(\S+)\s+(?:True|False)\s+(.+)$",
            line,
        )
        if not entry:
            continue
        reloc_type = normalize_macho_reloc_type(entry.group(1))
        target = classify_target("mach-o", entry.group(2))
        relocs.append((current_section, reloc_type, target))
    return sorted(set(relocs))


def parse_elf_sections(text: str) -> List[str]:
    sections: List[str] = []
    for raw_line in text.splitlines():
        match = re.match(r"^\s*\[\s*\d+\]\s+(\S+)\s+", raw_line)
        if match:
            sections.append(classify_section(match.group(1)))
    return sorted(set(sections))


def normalize_elf_reloc_type(reloc_type: str) -> str:
    mapping = {
        "R_X86_64_PC32": "pcrel32",
        "R_X86_64_PLT32": "branch32",
        "R_X86_64_64": "abs64",
        "R_X86_64_REX_GOTPCRELX": "gotpcrel",
        "R_X86_64_GOTPCREL": "gotpcrel",
    }
    return mapping.get(reloc_type, reloc_type.lower())


def parse_elf_relocations(text: str) -> List[Tuple[str, str, str]]:
    relocs: List[Tuple[str, str, str]] = []
    current_section = None
    for raw_line in text.splitlines():
        header = re.match(r"^Relocation section '([^']+)'", raw_line.strip())
        if header:
            raw_name = header.group(1)
            if raw_name.startswith(".rela"):
                raw_name = raw_name[5:]
            elif raw_name.startswith(".rel"):
                raw_name = raw_name[4:]
            current_section = classify_section(raw_name if raw_name.startswith(".") else f".{raw_name}")
            continue
        if current_section is None:
            continue
        line = raw_line.strip()
        if not line or line.startswith("Offset"):
            continue
        parts = line.split()
        if len(parts) < 5:
            continue
        reloc_type = normalize_elf_reloc_type(parts[2])
        target = classify_target("elf", parts[4])
        relocs.append((current_section, reloc_type, target))
    return sorted(set(relocs))


def dump_facts(path: pathlib.Path) -> List[str]:
    fmt = detect_format(path)
    facts = [f"object_format {fmt}"]

    nm_output = run_command(["nm", "-g", str(path)])
    defined, undefined = parse_nm_output(nm_output, fmt)
    for symbol in defined:
        facts.append(f"define {symbol}")
    for symbol in undefined:
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
        target_name = target
        if (
            target_name.startswith(PRIVATE_PREFIX)
            or target_name.startswith("section:eh_frame")
            or target_name.startswith("section:compact_unwind")
            or target_name.startswith("section:text")
            or section in {"compact_unwind", "eh_frame", "gcc_except_table"}
        ):
            facts.append(f"reloc {section} {reloc_type} {target_name}")

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

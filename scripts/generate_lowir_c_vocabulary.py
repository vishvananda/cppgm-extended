#!/usr/bin/env python3
"""Project the authoritative scaffold enums into the foreign C vocabulary."""
import argparse
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
ENUMS = {"TypeKind": "TYPE", "Opcode": "OP", "UnaryOp": "UNARY",
         "BinaryOp": "BINARY", "ComparePredicate": "CMP", "ConvertOp": "CONVERT",
         "ValueKind": "VALUE", "MemoryOrder": "ORDER", "IndexBounds": "BOUNDS",
         "VectorShape": "SHAPE", "VectorOp": "VECTOR"}


def generated():
    source = (ROOT / "dev/src/lowir_model.h").read_text()
    lines = ["#pragma once", "", "// Generated from lowir_model.h; run scripts/generate_lowir_c_vocabulary.py."]
    for name, prefix in ENUMS.items():
        body = re.search(r"enum class " + name + r" : std::uint8_t\s*\{(.*?)\};", source, re.S).group(1)
        body = re.sub(r"//[^\n]*", "", body)
        lines += ["", "enum Cppgm" + name, "{"]
        for number, entry in enumerate(body.split(",")):
            entry = entry.strip()
            if not entry:
                continue
            if not re.fullmatch(r"[A-Za-z][A-Za-z0-9]*", entry):
                raise ValueError("unsupported enum entry: " + entry)
            word = re.sub(r"([A-Z]+)([A-Z][a-z])", r"\1_\2", entry)
            word = re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", word).upper()
            lines.append(f"\tCPPGM_{prefix}_{word} = {number},")
        lines[-1] = lines[-1].rstrip(",")
        lines.append("};")
    return "\n".join(lines) + "\n"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    destination = ROOT / "dev/src/lowir_c_vocabulary.h"
    content = generated()
    if args.check:
        if destination.read_text() != content:
            raise SystemExit("stale C vocabulary; run scripts/generate_lowir_c_vocabulary.py")
    else:
        destination.write_text(content)


if __name__ == "__main__":
    main()

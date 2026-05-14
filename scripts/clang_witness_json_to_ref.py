#!/usr/bin/env python3

import argparse
import importlib.util
import json
import pathlib
import sys


ROOT = pathlib.Path(__file__).resolve().parents[1]
RENDERER = ROOT / "validation" / "templates" / "emit_templates_to_witness.py"


def load_renderer():
    spec = importlib.util.spec_from_file_location(
        "emit_templates_to_witness", RENDERER)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"unable to load witness renderer from {RENDERER}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def read_json(path_text):
    if path_text == "-":
        return json.loads(sys.stdin.read())
    return json.loads(pathlib.Path(path_text).read_text(encoding="utf-8"))


def main():
    parser = argparse.ArgumentParser(
        description="Render patched-Clang template witness JSON as .ref.witness text.")
    parser.add_argument("json", help="Patched-Clang witness JSON path, or '-' for stdin")
    parser.add_argument("-o", "--output", help="Output .ref.witness path; defaults to stdout")
    parser.add_argument("--debug", action="store_true",
                        help="Render the debug witness projection")
    args = parser.parse_args()

    renderer = load_renderer()
    document = read_json(args.json)
    if args.debug:
        text = renderer.render_emit_templates_debug_text(document)
    else:
        text = renderer.render_emit_templates_text(document)

    if args.output:
        pathlib.Path(args.output).write_text(text, encoding="utf-8")
    else:
        sys.stdout.write(text)
    return 0


if __name__ == "__main__":
    sys.exit(main())

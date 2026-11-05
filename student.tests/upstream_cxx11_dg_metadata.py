#!/usr/bin/env python3
"""Extract GCC directives with DejaGnu's own parser, retaining unresolved cases.

This does not evaluate target predicates or execute tests. It preserves every
native directive word and source line for the external adapter's next stage.
Old-Deja/default-driver inputs with no dg directives remain explicitly unparsed.
"""
import argparse
import base64
from collections import Counter
import hashlib
import json
from pathlib import Path
import subprocess


DRIVER = r'''
set library [lindex $argv 0]
set inputs [lindex $argv 1]
# dg-get-options uses verbose only for logging; no test predicates are stubbed.
proc verbose {args} {}
source [file join $library utils.exp]
source [file join $library dg.exp]
proc encode {value} { return [binary encode base64 -maxlen 0 [encoding convertto utf-8 $value]] }
puts "V\t[encode [info patchlevel]]"
set stream [open $inputs r]
while {[gets $stream encoded] >= 0} {
    set path [encoding convertfrom utf-8 [binary decode base64 $encoded]]
    if {[catch {
        set directives [dg-get-options $path]
        if {[llength $directives] == 0} { puts "E\t[encode $path]" }
        foreach directive $directives {
            set words {}
            foreach word $directive { lappend words [encode $word] }
            puts "D\t[encode $path]\t[join $words \t]"
        }
    } failure]} { puts "X\t[encode $path]\t[encode $failure]" }
}
close $stream
'''


def decode(value):
    return base64.b64decode(value).decode()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--inventory", type=Path, required=True)
    parser.add_argument("--sources", type=Path, required=True)
    parser.add_argument("--dejagnu-library", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    cases = {}
    with args.inventory.open() as stream:
        for line in stream:
            row = json.loads(line)
            if row["repository"] != "gcc" or row["role"] != "candidate" or Path(row["path"]).suffix not in {".C", ".cc", ".cpp", ".cxx", ".c"}:
                continue
            # A symlinked testcase still has its own upstream identity and driver.
            path = (args.sources / "gcc" / row["path"]).absolute()
            if hashlib.sha256(path.read_bytes()).hexdigest() != row.get("sha256"):
                raise ValueError(f"source differs from pinned inventory: {path}")
            cases[str(path)] = dict(path=row["path"], sha256=row["sha256"], git_blob=row["git_blob"],
                                    revision=row["revision"], raw_markers=row.get("directives", []),
                                    directives=[], status="awaiting-native-parser")
    inputs = args.output / "inputs.b64"
    inputs.write_text("".join(base64.b64encode(p.encode()).decode() + "\n" for p in cases))
    driver = args.output / "extract.tcl"
    driver.write_text(DRIVER)
    native = args.output / "native.tsv"
    with native.open("w") as out, (args.output / "stderr").open("w") as err:
        subprocess.run(["tclsh", str(driver.resolve()), str(args.dejagnu_library.resolve()), str(inputs.resolve())],
                       stdout=out, stderr=err, check=True)
    version = None
    for line in native.read_text().splitlines():
        fields = line.split("\t")
        if fields[0] == "V":
            version = decode(fields[1])
            continue
        case = cases[decode(fields[1])]
        if fields[0] == "D":
            words = list(map(decode, fields[2:]))
            case["directives"].append(dict(command=words[0], line=int(words[1]), arguments=words[2:]))
            case["status"] = "native-directives-extracted; applicability-unresolved"
        elif fields[0] == "E":
            case["status"] = "native-parser-empty; driver-default-or-old-deja-unresolved"
        else:
            case.update(status="native-parser-error", error=decode(fields[2]))
    counts = Counter(case["status"] for case in cases.values())
    with (args.output / "metadata.jsonl").open("w") as stream:
        for case in cases.values():
            stream.write(json.dumps(case) + "\n")
    manifest = dict(tcl_version=version, count=len(cases), counts=counts,
                    inventory_sha256=hashlib.sha256(args.inventory.read_bytes()).hexdigest(),
                    native_libraries={name: hashlib.sha256((args.dejagnu_library / name).read_bytes()).hexdigest()
                                      for name in ["utils.exp", "dg.exp"]})
    (args.output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
    print(json.dumps(manifest, indent=2))


if __name__ == "__main__":
    main()

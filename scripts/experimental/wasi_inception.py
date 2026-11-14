#!/usr/bin/env python3
"""Maintainer-only, frozen-input WASI compiler inception (no student gate)."""
import argparse
import concurrent.futures
import hashlib
import json
from pathlib import Path
import re
import resource
import shutil
import subprocess
import sys
import time

HERE = Path(__file__).resolve().parent
REPO = HERE.parent.parent


def digest(data):
    return hashlib.sha256(data).hexdigest()


def save(path, value):
    path.write_text(json.dumps(value, indent=2) + "\n")


def copy_tree(source, destination, exclude=()):
    for source_file in sorted(source.rglob("*")):
        relative = source_file.relative_to(source)
        if any(part in exclude for part in relative.parts) or not source_file.is_file():
            continue
        target = destination / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(source_file, target)


def floating_cases():
    lines = [
        "long double rounding_boundary = 0x1.0000000000000002p0L * 0x1.8000000000000002p0L;",
        "long double minimum_subnormal = 0x1p-16445L;",
        "long double maximum_subnormal = 0x0.fffffffffffffffep-16382L;",
        "long double integer_boundary = static_cast<long double>((static_cast<unsigned __int128>(1) << 127) + (static_cast<unsigned __int128>(1) << 63) + 1);",
    ]
    for typ, suffix in [("float", "f"), ("double", ""), ("long double", "l")]:
        values = ["0.0", "-0.0", "1.0", "-1.0", "__builtin_inf" + suffix + "()",
                  "-__builtin_inf" + suffix + "()", "__builtin_nan" + suffix + '("0x123")',
                  "-__builtin_nan" + suffix + '("0x321")', "__builtin_nans" + suffix + '("0x111")']
        values = [v if "builtin" in v else v + ("L" if suffix == "l" else suffix) for v in values]
        for i, left in enumerate(values):
            for j, right in enumerate(values):
                for k, op in enumerate("+-*/"):
                    lines.append(f"{typ} special_{suffix or 'd'}_{i}_{j}_{k} = ({left}) {op} ({right});")
    state = 0x18c76abd
    def next_word():
        nonlocal state
        state ^= state << 13
        state &= (1 << 64) - 1
        state ^= state >> 7
        state ^= state << 17
        state &= (1 << 64) - 1
        return state
    for i in range(256):
        literals = []
        for _ in range(2):
            mantissa = next_word() | (1 << 63)
            exponent = next_word() % 2001 - 1063
            sign = "-" if next_word() & 1 else ""
            literals.append(f"{sign}0x{mantissa:016x}p{exponent}L")
        for k, op in enumerate("+-*/"):
            lines.append(f"long double finite_{i}_{k} = ({literals[0]}) {op} ({literals[1]});")
    return "\n".join(lines) + "\n"


def source_sets(tools):
    """Use the compiler's owning source lists, including each tool's entry point."""
    makefile = "include dev/frontend_source_sets.mk\nlist:\n"
    for name in tools:
        makefile += "\t@printf '%s\\n' '" + name + "' $(FRONTEND_OBJ_BASENAMES_" + name + ") END\n"
    lines = subprocess.check_output(["make", "-s", "-f", "-", "list"],
                                    input=makefile.encode(), cwd=REPO).decode().splitlines()
    result = {}
    while lines:
        name = lines.pop(0)
        end = lines.index("END")
        result[name] = ["dev/src/" + source + ".cpp" for source in lines[:end]] + ["dev/" + name + ".cpp"]
        lines = lines[end + 1:]
    return result


def prepare(args):
    package = args.directory.resolve()
    package.mkdir()  # Never replace an existing experiment or its observations.
    inputs = package / "inputs"
    inputs.mkdir()
    copy_tree(REPO / "dev/src", inputs / "dev/src")
    tools = getattr(args, "reference_tools", ["cppgm++"])
    for name in tools:
        shutil.copyfile(REPO / ("dev/" + name + ".cpp"), inputs / ("dev/" + name + ".cpp"))
    for name in ['LICENSE', 'NOTICE']:
        shutil.copyfile(REPO / name, inputs / name)
    shutil.copyfile(HERE / "wasi_host.h", inputs / "wasi_host.h")
    tool_sources = source_sets(tools)
    sources = tool_sources["cppgm++"]
    config = (REPO / "obj/generated/cppgm_builtin_host_config.h").read_text()
    paths = re.search(r"kStandardIncludePaths\[\] = \{(.*?)\};", config, re.S)
    if paths is None:
        raise RuntimeError("generated host configuration has no include path table")
    host_paths = re.findall(r'"([^"\n]+)"', paths.group(1))
    # The default output target stays x86-64. Preserve its predefined macros,
    # and freeze its headers at package-relative paths, independent of the host.
    x86_paths = ["x86" + name for name in host_paths]
    config = config[:paths.start(1)] + "\n" + "".join(
        '  "' + name + '",\n' for name in x86_paths) + "  nullptr\n" + config[paths.end(1):]
    (inputs / "generated").mkdir()
    (inputs / "generated/cppgm_builtin_host_config.h").write_text(config)
    if args.x86_headers:
        for name in host_paths:
            if Path(name).is_dir():
                copy_tree(Path(name), inputs / ("x86" + name))
    sysroot = args.sdk / "share/wasi-sysroot"
    copy_tree(sysroot / "include/wasm32-wasip1", inputs / "sysroot/include/wasm32-wasip1",
              exclude=("noeh",))
    for name in ["crt1-command.o", "libc.a", "libc-printscan-long-double.a",
                 "eh/libc++.a", "eh/libc++abi.a", "eh/libunwind.a"]:
        dest = inputs / "sysroot/lib/wasm32-wasip1" / name
        dest.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(sysroot / "lib/wasm32-wasip1" / name, dest)
    resource = args.sdk / "lib/clang/23"
    copy_tree(resource / "include", inputs / "resource/include")
    dest = inputs / "resource/lib/wasm32-unknown-wasip1/libclang_rt.builtins.a"
    dest.parent.mkdir(parents=True)
    shutil.copyfile(resource / "lib/wasm32-unknown-wasip1/libclang_rt.builtins.a", dest)
    shutil.copyfile(args.compiler, package / "native-compiler")
    (package / "native-compiler").chmod(0o755)
    shutil.copytree(args.shim / "dist", package / "shim")
    for name in ["package.json", "LICENSE-MIT", "LICENSE-APACHE"]:
        source = args.shim / name
        if source.is_file():
            shutil.copyfile(source, package / "shim" / name)
    shim_info = json.loads((args.shim / "package.json").read_text())
    if shim_info["name"] != "@bjorn3/browser_wasi_shim" or shim_info["version"] != "0.4.2":
        raise RuntimeError("expected browser_wasi_shim 0.4.2")
    (inputs / "checks").mkdir()
    (inputs / "checks/floating.cpp").write_text(floating_cases())
    flags = ["--target=wasm32-wasip1", "--sysroot=sysroot", "-resource-dir=resource",
             "-std=c++11", "-D_GNU_SOURCE", "-include", "wasi_host.h",
             "-D__cpp_exceptions=199711L", "-D__cpp_rtti=199711L",
             "-DCPPGM_TELEMETRY_ENABLED=0", "-Idev/src", "-Igenerated", "-O" + str(args.optimization)]
    files = []
    with (package / "bundle.bin").open("wb") as bundle:
        for path in sorted(inputs.rglob("*")):
            if path.is_file():
                data = path.read_bytes()
                files.append({"path": path.relative_to(inputs).as_posix(), "offset": bundle.tell(),
                              "size": len(data), "sha256": digest(data)})
                bundle.write(data)
    runners = ["wasi_inception.py", "wasi_worker.mjs", "wasi_runtime.mjs",
               "wasi_browser.html", "wasi_browser_test.cjs", "transport_bytes.mjs"]
    for name in runners:
        shutil.copyfile(HERE / name, package / name)
    manifest = {"schema": 1, "sources": sources, "flags": flags,
                "link_flags": ["-Wl,-z,stack-size=8388608", "-lc-printscan-long-double"], "files": files,
                "x86_flags": ["--target=x86_64-linux-gnu", "-std=c++11", "-D_GNU_SOURCE",
                              "-DCPPGM_TELEMETRY_ENABLED=0", "-nostdinc", "-Idev/src", "-Igenerated"]
                             + [flag for name in x86_paths for flag in ["-isystem", name]],
                "x86_headers": args.x86_headers, "x86_sources": sources + ["checks/floating.cpp"],
                "native_sha256": digest((package / "native-compiler").read_bytes()),
                "bundle_sha256": digest((package / "bundle.bin").read_bytes()),
                "shim_files": {p.relative_to(package).as_posix(): digest(p.read_bytes())
                               for p in sorted((package / "shim").rglob("*")) if p.is_file()},
                "runner_files": {name: digest((package / name).read_bytes()) for name in runners},
                "sdk_version": (args.sdk / "VERSION").read_text(),
                "revision": subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=REPO).decode().strip(),
                "tool_sources": tool_sources,
                "dirty": bool(subprocess.check_output(["git", "status", "--porcelain"], cwd=REPO)),
                "source_sha256": digest(json.dumps([(f['path'], f['sha256']) for f in files
                                                     if f['path'].startswith('dev/')]).encode())}
    save(package / "manifest.json", manifest)
    (package / "manifest.sha256").write_text(digest((package / "manifest.json").read_bytes()) + "\n")
    print(f"Frozen {len(sources)} sources, {len(files)} files, {(package / 'bundle.bin').stat().st_size} bytes", flush=True)


def verify(package, manifest):
    if digest((package / "manifest.json").read_bytes()) != (package / "manifest.sha256").read_text().strip():
        raise RuntimeError("manifest changed")
    for item in manifest["files"]:
        path = package / "inputs" / item["path"]
        if digest(path.read_bytes()) != item["sha256"]:
            raise RuntimeError("input changed: " + item["path"])
    if digest((package / "native-compiler").read_bytes()) != manifest["native_sha256"]:
        raise RuntimeError("native compiler changed")
    if digest((package / "bundle.bin").read_bytes()) != manifest["bundle_sha256"]:
        raise RuntimeError("input bundle changed")
    for name, expected in {**manifest["shim_files"], **manifest["runner_files"]}.items():
        if digest((package / name).read_bytes()) != expected:
            raise RuntimeError("runner or shim changed: " + name)


def object_name(source):
    return source.replace("/", "__") + ".o"


def compare(expected, actual):
    a, b = expected.read_bytes(), actual.read_bytes()
    if a != b:
        offset = next((i for i, (x, y) in enumerate(zip(a, b)) if x != y), min(len(a), len(b)))
        raise RuntimeError(f"byte mismatch {actual.name} at {offset}: {len(a)} versus {len(b)} bytes")
    return {"bytes": len(b), "sha256": digest(b)}


def build(args):
    package = args.directory.resolve()
    manifest = json.loads((package / "manifest.json").read_text())
    verify(package, manifest)
    inputs = package / "inputs"
    runtime = subprocess.check_output([str(args.wasmtime), "--version"], text=True).strip()
    for stage in args.stages:
        stage_dir = inputs / ("w" + str(stage))
        stage_dir.mkdir()  # Partial stages are never silently reused.
        if stage == 1:
            runner = [str(package / "native-compiler")]
        else:
            previous = inputs / ("w" + str(stage - 1)) / "cppgm.wasm"
            compiled = package / ("w" + str(stage - 1) + ".cwasm")
            subprocess.run([str(args.wasmtime), "compile", "-W", "exceptions=y", "-W",
                            "max-wasm-stack=8388608", "-o", str(compiled), str(previous)], check=True)
            runner = [str(args.wasmtime), "run", "--allow-precompiled", "-W", "exceptions=y", "-W",
                      "max-wasm-stack=8388608", "--dir", str(inputs) + "::.", str(compiled)]
        stage_start = time.monotonic()

        def run(arguments, output):
            command = runner + manifest["flags"] + arguments + ["-o", output]
            started = time.monotonic()
            with (inputs / (output + ".log")).open("wb") as log:
                result = subprocess.run(command, cwd=inputs, stdout=log, stderr=log)
            row = {"command": command, "exit": result.returncode, "seconds": time.monotonic() - started,
                   "output": output}
            if result.returncode:
                raise RuntimeError(f"compiler failed: {output}; see {output}.log")
            row.update({"bytes": (inputs / output).stat().st_size, "sha256": digest((inputs / output).read_bytes())})
            if stage > 1:
                compare(inputs / ("w" + str(stage - 1)) / Path(output).name, inputs / output)
            print(json.dumps(row), flush=True)
            return row

        def compile_source(source):
            return run(["-c", source], stage_dir.name + "/" + object_name(source))

        rows, failures = [], []
        with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
            pending = {pool.submit(compile_source, source): source for source in manifest["sources"]}
            for future in concurrent.futures.as_completed(pending):
                try:
                    rows.append(future.result())
                except Exception as error:
                    failures.append({"source": pending[future], "error": str(error)})
        if failures:
            save(package / (stage_dir.name + "-failed.json"), {"passed": rows, "failures": failures})
            raise RuntimeError(f"{len(failures)} source compilations failed; see {stage_dir.name}-failed.json")
        rows.sort(key=lambda row: row["output"])
        objects = [stage_dir.name + "/" + object_name(source) for source in manifest["sources"]]
        rows.append(run(objects + manifest["link_flags"], stage_dir.name + "/cppgm.wasm"))
        record = {"stage": stage, "seconds": time.monotonic() - stage_start, "commands": rows,
                  "manifest": digest((package / "manifest.json").read_bytes()), "runtime": runtime,
                  "cumulative_peak_child_rss_bytes": resource.getrusage(resource.RUSAGE_CHILDREN).ru_maxrss
                      * (1 if sys.platform == "darwin" else 1024)}
        save(package / (stage_dir.name + "-complete.json"), record)
        print(f"Completed W{stage}: {record['seconds']:.3f}s", flush=True)
    verify(package, manifest)


def x86(args):
    package = args.directory.resolve()
    manifest = json.loads((package / "manifest.json").read_text())
    verify(package, manifest)
    if not manifest["x86_headers"]:
        raise RuntimeError("prepare with --x86-headers first")
    inputs = package / "inputs"
    out = inputs / "x86-check"
    out.mkdir()
    module = inputs / "w1/cppgm.wasm"
    compiled = package / "x86-compiler.cwasm"
    subprocess.run([str(args.wasmtime), "compile", "-W", "exceptions=y", "-W",
                    "max-wasm-stack=8388608", "-o", str(compiled), str(module)], check=True)
    wasi = [str(args.wasmtime), "run", "--allow-precompiled", "-W", "exceptions=y", "-W",
            "max-wasm-stack=8388608", "--dir", str(inputs) + "::.", str(compiled)]
    started = time.monotonic()

    def check(task):
        source, level = task
        name = object_name(source) + ".O" + str(level)
        flags = manifest["x86_flags"] + ["-O" + str(level), "-c", source]
        results = []
        for host, runner in [("native", [str(package / "native-compiler")]), ("wasi", wasi)]:
            output = "x86-check/" + name + "." + host + ".o"
            command = runner + flags + ["-o", output]
            before = time.monotonic()
            with (inputs / (output + ".log")).open("wb") as log:
                result = subprocess.run(command, cwd=inputs, stdout=log, stderr=log)
            results.append({"command": command, "exit": result.returncode,
                            "seconds": time.monotonic() - before, "output": output})
            if result.returncode:
                raise RuntimeError(f"x86 compile failed: {output}; see {output}.log")
        identity = compare(inputs / results[0]["output"], inputs / results[1]["output"])
        row = {"source": source, "level": level, "commands": results, **identity}
        print(json.dumps(row), flush=True)
        return row

    tasks = [(source, level) for level in args.levels for source in manifest["x86_sources"]]
    rows, failures = [], []
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
        pending = {pool.submit(check, task): task for task in tasks}
        for future in concurrent.futures.as_completed(pending):
            try:
                rows.append(future.result())
            except Exception as error:
                failure = {"task": pending[future], "error": str(error)}
                failures.append(failure)
                print(json.dumps(failure), flush=True)
    if failures:
        save(package / "x86-failed.json", {"passed": rows, "failures": failures})
        raise RuntimeError(f"{len(failures)} x86 comparisons failed; see x86-failed.json")
    rows.sort(key=lambda row: (row["level"], row["source"]))
    save(package / "x86-complete.json", {"seconds": time.monotonic() - started, "objects": rows})
    verify(package, manifest)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    pack = commands.add_parser("prepare")
    pack.add_argument("--directory", required=True, type=Path)
    pack.add_argument("--sdk", required=True, type=Path)
    pack.add_argument("--compiler", type=Path, default=REPO / "dev/cppgm++")
    pack.add_argument("--shim", required=True, type=Path)
    pack.add_argument("--x86-headers", action="store_true")
    pack.add_argument("--optimization", type=int, choices=[0, 1, 2, 3], default=3)
    run = commands.add_parser("build")
    run.add_argument("--directory", required=True, type=Path)
    run.add_argument("--wasmtime", required=True, type=Path)
    run.add_argument("--jobs", type=int, default=1)
    run.add_argument("--stages", nargs="+", type=int, choices=[1, 2, 3], default=[1, 2, 3])
    cross = commands.add_parser("x86")
    cross.add_argument("--directory", required=True, type=Path)
    cross.add_argument("--wasmtime", required=True, type=Path)
    cross.add_argument("--jobs", type=int, default=1)
    cross.add_argument("--levels", nargs="+", type=int, choices=[0, 1, 2, 3], default=[0, 2])
    args = parser.parse_args()
    {"prepare": prepare, "build": build, "x86": x86}[args.command](args)


if __name__ == "__main__":
    main()

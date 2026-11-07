#!/usr/bin/env python3
"""Strict compiler boundary for the upstream GCC dg adapter.

This is a maintainer tool. Native dg owns expectations; this process records
compiler outcomes independently so an expected error cannot hide a crash.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import resource
import signal
import subprocess
import sys
import time
import uuid


PRESENTATION = {"-fdiagnostics-plain-output", "-fmessage-length=0",
                "-Wno-complain-wrong-lang"}
STAGES = {"-c", "-S", "-E", "-fsyntax-only"}
SOURCE_SUFFIXES = {".C", ".cc", ".cpp", ".cxx", ".c", ".i", ".ii"}
RESOURCE_ERROR = re.compile(
    r"^(?:ERROR: )?(?:std::bad_alloc|std::bad_array_new_length|"
    r"terminate called|Assertion .* failed)|"
    r"^(?:[^\n]*:\s*)?internal compiler error\b", re.M | re.I)


class Blocked(ValueError):
    pass


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def write_json(path, value):
    temporary = path.with_suffix(".pending")
    temporary.write_text(json.dumps(value, indent=2) + "\n")
    temporary.replace(path)


def normalize_diagnostics(stderr):
    return re.sub(r"^ERROR: (.*):(\d+):(\d+): (.*)$",
                  r"\1:\2:\3: error: \4", stderr, flags=re.M)


def translate(args, config):
    """Fail closed: driver acceptance of an arbitrary -f/-W is insufficient."""
    dialect = config["dialect"]
    if config["mode"] == "host":
        # Preserve the host's option/library ordering exactly.
        for arg in args:
            if arg.startswith("-std=") and arg[5:] not in {
                    dialect, "c++0x" if dialect == "c++11" else "gnu++0x"}:
                raise Blocked("dialect outside selected configuration: " + arg)
        selected = list(args)
        if not any(arg.startswith("-std=") for arg in selected):
            selected.insert(0, "-std=" + dialect)
        return selected, [], [], [], []
    translated, adaptations, sources, objects, link_flags = [], [], [], [], []
    index = 0
    while index < len(args):
        arg = args[index]
        index += 1
        if arg.startswith("-std="):
            standard = {"c++0x": "c++11", "gnu++0x": "gnu++11"}.get(arg[5:], arg[5:])
            if standard != dialect:
                raise Blocked("dialect outside selected configuration: " + arg)
            translated.append("-std=" + standard)
        elif arg in {"-o", "-I", "-D", "-U", "-isystem", "-include", "-x", "-L", "-l"}:
            if index == len(args):
                raise Blocked("missing operand: " + arg)
            operand = args[index]
            index += 1
            if arg == "-x" and operand != "c++":
                raise Blocked("unsupported language: " + operand)
            if arg in {"-L", "-l"}:
                link_flags.extend([arg, operand])
            translated.extend([arg, operand])
        elif arg == "-dumpbase":
            if index == len(args):
                raise Blocked("missing operand: " + arg)
            index += 1
            adaptations.append("omit native dump basename; dump oracle unavailable")
        elif arg in PRESENTATION:
            adaptations.append("omit diagnostic presentation option " + arg)
        elif arg == "-S":
            translated.append("-c")
            adaptations.append("assembly stage replaced by object; no assembly oracle")
        elif arg in {"-c", "-E", "-fsyntax-only", "-O0", "-O1", "-O2", "-O3", "-nostdinc"}:
            translated.append(arg)
        elif arg.startswith(("-I", "-D", "-U", "-isystem")):
            translated.append(arg)
        elif arg.startswith(("-L", "-l")):
            link_flags.append(arg)
            translated.append(arg)
        elif arg.startswith("-"):
            raise Blocked("unverified compiler option: " + arg)
        else:
            translated.append(arg)
        if not arg.startswith("-"):
            if Path(arg).suffix in SOURCE_SUFFIXES:
                sources.append(arg)
            elif Path(arg).suffix in {".o", ".a", ".so"}:
                objects.append(arg)
            else:
                raise Blocked("unrecognized input kind: " + arg)
    if not any(arg.startswith("-std=") for arg in translated):
        translated.insert(0, "-std=" + dialect)
        adaptations.append("select dialect for native capability probe")
    return translated, adaptations, sources, objects, link_flags


def execute(command, prefix, config):
    """Bound the subprocess, retain raw streams and kill its entire process group."""
    def limits():
        resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
        for name, number in (("cpu_seconds", resource.RLIMIT_CPU),
                             ("address_space_bytes", resource.RLIMIT_AS),
                             ("file_bytes", resource.RLIMIT_FSIZE)):
            value = config[name]
            resource.setrlimit(number, (value, value + 1 if name == "cpu_seconds" else value))
    start = time.monotonic()
    result = {"command": command, "classification": "started"}
    stdout_path, stderr_path = Path(str(prefix) + ".stdout"), Path(str(prefix) + ".stderr")
    with stdout_path.open("wb") as out, stderr_path.open("wb") as err:
        process = subprocess.Popen(command, stdout=out, stderr=err,
                                   start_new_session=True, preexec_fn=limits)
        try:
            try:
                process.wait(timeout=config["timeout_seconds"])
                result["classification"] = "success" if process.returncode == 0 else "rejection"
            except subprocess.TimeoutExpired:
                result["classification"] = "timeout"
        finally:
            try:
                os.killpg(process.pid, signal.SIGKILL)
            except ProcessLookupError:
                pass
            process.wait()
    stdout = stdout_path.read_text(errors="replace")
    stderr = stderr_path.read_text(errors="replace")
    compiler_error = config.get("compiler_error_scan", True) and (
        process.returncode == 70 or process.returncode >= 128 or RESOURCE_ERROR.search(stderr))
    if result["classification"] != "timeout" and (process.returncode < 0 or compiler_error):
        result["classification"] = "abnormal"
    result.update(returncode=process.returncode, elapsed_seconds=time.monotonic() - start,
                  stdout_path=str(stdout_path), stderr_path=str(stderr_path),
                  stdout_sha256=sha256(stdout_path), stderr_sha256=sha256(stderr_path))
    return result, stdout, stderr


def compile_commands(args, config, prefix):
    translated, adaptations, sources, objects, link_flags = translate(args, config)
    if config["mode"] == "host":
        return [[config["host"], *translated, *link_flags]], adaptations
    if not sources and not objects:
        raise Blocked("no supported source/object input")
    if any(arg in args for arg in STAGES):
        if link_flags:
            raise Blocked("link options in a non-link stage")
        return [[config["candidate"], *translated]], adaptations
    # Compile every TU with cppgm; delegate only the final hosted link.
    output = "a.out"
    flags, link_inputs, commands = [], [], []
    index = 0
    while index < len(translated):
        arg = translated[index]
        index += 1
        if arg == "-o":
            output = translated[index]
            index += 1
        elif arg in {"-I", "-D", "-U", "-isystem", "-include", "-x"}:
            flags.extend([arg, translated[index]])
            index += 1
        elif arg in {"-L", "-l"}:
            link_inputs.extend([arg, translated[index]])
            index += 1
        elif arg.startswith(("-L", "-l")) or arg in objects:
            link_inputs.append(arg)
        elif arg in sources:
            obj = str(prefix) + ".tu" + str(len(link_inputs)) + ".o"
            link_inputs.append(obj)
            commands.append((arg, obj))
        else:
            flags.append(arg)
    commands = [[config["candidate"], *flags, "-c", source, "-o", obj]
                for source, obj in commands]
    commands.append([config["host"], *link_inputs, "-o", output])
    adaptations.append("cppgm objects; hosted link through " + config["host"])
    return commands, adaptations


def run_proxy(args, config):
    directory = Path(config["log"])
    directory.mkdir(parents=True, exist_ok=True)
    prefix = directory / uuid.uuid4().hex
    record_path = prefix.with_suffix(".json")
    record = dict(incoming=args, cwd=os.getcwd(), started_at_ns=time.time_ns(), pid=os.getpid(),
                  phase=os.environ.get("CPPGM_DG_PHASE", "initialization"),
                  classification="started", steps=[], adaptations=[])
    write_json(record_path, record)
    stdout, stderr, code = "", "", 80
    try:
        query = len(args) == 1 and (args[0] in {"--version", "-dumpmachine", "-dumpversion",
                                               "-print-multi-directory", "-print-multi-lib"} or
                                  args[0].startswith("-print-file-name="))
        if query:
            record["role"] = "query"
            if args == ["-print-multi-directory"]:
                stdout = ".\n"
            elif args == ["-print-multi-lib"]:
                stdout = ".;\n"
            else:
                tool = config["candidate"] if args == ["--version"] and config["mode"] != "host" else config["host"]
                step, stdout, stderr = execute([tool, *args], prefix, config)
                record["steps"].append(step)
                record["classification"] = step["classification"]
            if record["classification"] == "started":
                record["classification"] = "success"
            record["query_policy"] = "single x86-64 multilib; host runtime paths/target; candidate version"
            code = 0 if record["classification"] == "success" else 70
        else:
            sources = [Path(arg).resolve() for arg in args if Path(arg).is_file()
                       and Path(arg).suffix in SOURCE_SUFFIXES]
            record["role"] = "test" if any(Path(config["source_root"]).resolve() in p.parents for p in sources) else "probe"
            record["probe_oracle"] = os.environ.get("CPPGM_DG_PROBE_ORACLE")
            record["sources"] = []
            for index, source in enumerate(sources):
                saved = Path(str(prefix) + ".input" + str(index) + source.suffix)
                saved.write_bytes(source.read_bytes())
                record["sources"].append(dict(path=str(source), sha256=sha256(source), saved=str(saved)))
            if (config["mode"] != "host" and record["role"] == "probe" and "-S" in args and
                    record["probe_oracle"] != "no-messages"):
                raise Blocked("assembly-inspecting or unknown probe interface unavailable")
            commands, record["adaptations"] = compile_commands(args, config, prefix)
            for index, command in enumerate(commands):
                step, stdout, stderr = execute(command, Path(str(prefix) + ".step" + str(index)), config)
                record["steps"].append(step)
                record["classification"] = step["classification"]
                if step["classification"] != "success":
                    break
                if "-o" in command and "-E" not in command and "-fsyntax-only" not in command:
                    output = Path(command[command.index("-o") + 1])
                    if not output.is_file():
                        record["classification"] = "abnormal"
                        stderr += "\nadapter: successful compiler produced no requested output\n"
                        break
                    data = output.read_bytes()
                    saved = Path(str(prefix) + ".output" + str(index))
                    saved.write_bytes(data)
                    step["output"] = dict(path=str(output), saved=str(saved), sha256=sha256(saved), bytes=len(data))
                    if config["mode"] != "host" and not data.startswith(b"\x7fELF"):
                        record["classification"] = "abnormal"
                        stderr += "\nadapter: expected an ELF object/executable\n"
                        break
            code = 0 if record["classification"] == "success" else 1
            if record["classification"] in {"abnormal", "timeout"}:
                code = 70
    except Blocked as error:
        record.update(classification="blocked", reason=str(error))
        stderr = "adapter unavailable: " + str(error) + "\n"
    except Exception as error:
        record.update(classification="adapter-error", reason=repr(error))
        stderr = "adapter internal failure: " + repr(error) + "\n"
        code = 70
    record.update(exitcode=code, stderr=stderr, stdout=stdout)
    if config["mode"] != "host":
        stderr = normalize_diagnostics(stderr)
    record["normalized_stderr"] = stderr
    write_json(record_path, record)
    sys.stdout.write(stdout)
    sys.stderr.write(stderr)
    return code


def run_executable(args, config):
    directory = Path(config["log"])
    prefix = directory / uuid.uuid4().hex
    record_path = prefix.with_suffix(".json")
    program = os.environ.get("CPPGM_DG_EXECUTABLE", "")
    record = dict(role="runtime", phase="test", incoming=[program, *args],
                  classification="started", started_at_ns=time.time_ns(), steps=[])
    write_json(record_path, record)
    try:
        record["executable_sha256"] = sha256(program)
        step, stdout, stderr = execute([program, *args], prefix, dict(config, compiler_error_scan=False))
        record.update(classification=step["classification"], steps=[step])
        code = 70 if step["classification"] in {"abnormal", "timeout"} else step["returncode"]
    except Exception as error:
        record.update(classification="adapter-error", reason=repr(error))
        stdout, stderr, code = "", "runtime adapter failure: " + repr(error) + "\n", 70
    record.update(exitcode=code, stdout=stdout, stderr=stderr)
    write_json(record_path, record)
    sys.stdout.write(stdout)
    sys.stderr.write(stderr)
    return code


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--config", type=Path, required=True)
    parser.add_argument("--runtime", action="store_true")
    parser.add_argument("arguments", nargs=argparse.REMAINDER)
    args = parser.parse_args()
    def interrupted(number, frame):
        # Let execute's finally block kill the child session on PTY HUP/TERM.
        raise RuntimeError("adapter interrupted by signal " + str(number))
    for number in (signal.SIGHUP, signal.SIGTERM, signal.SIGINT):
        signal.signal(number, interrupted)
    arguments = args.arguments[1:] if args.arguments[:1] == ["--"] else args.arguments
    config = json.loads(args.config.read_text())
    return run_executable(arguments, config) if args.runtime else run_proxy(arguments, config)


if __name__ == "__main__":
    sys.exit(main())

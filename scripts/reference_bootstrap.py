#!/usr/bin/env python3
"""PA34 reference bootstrap and PA35 constant-source builds (no Rust dependency)."""
import argparse
import contextlib
import fcntl
import hashlib
import io
import json
import os
from pathlib import Path
import re
import shutil
import signal
import subprocess
import sys
import tarfile

TOOLS = ("pptoken", "posttoken", "ppexpr", "preproc", "cppgm++", "abimangle",
         "lowir", "lowiropt", "lowir2native")
PROTOCOL = 1


def sha(path):
    with Path(path).open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def digest(value):
    return hashlib.sha256(json.dumps(value, sort_keys=True).encode()).hexdigest()


def write_json(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(".tmp")
    temporary.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")
    temporary.replace(path)


def environment(jobs):
    env = {k: v for k, v in os.environ.items()
           if not k.startswith(("CPPGM_", "MAKE", "INCEPTION_", "LD_"))
           and k not in ("CXX", "CC", "CPPFLAGS", "CFLAGS", "CXXFLAGS", "LDFLAGS", "LDLIBS",
                         "CPATH", "CPLUS_INCLUDE_PATH", "LIBRARY_PATH", "COMPILER_PATH", "GCC_EXEC_PREFIX")}
    env.update(LC_ALL="C", TZ="UTC", CPPGM_TEST_JOBS=str(jobs),
               CPPGM_BATCH_TESTS="0", CPPGM_LOWIR_DIRECT_TEXT_COMPARE="1", KEEP_GOING="1")
    return env


def capture(command):
    return subprocess.check_output(command, stderr=subprocess.STDOUT, env=environment(1))


def execute(command, cwd, log, env, timeout=21600):
    log.parent.mkdir(parents=True, exist_ok=True)
    print("reference bootstrap: " + " ".join(map(str, command)), flush=True)
    with log.open("wb") as stream:
        process = subprocess.Popen(list(map(str, command)), cwd=cwd, stdout=stream,
                                   stderr=subprocess.STDOUT, env=env, start_new_session=True)
        try:
            return process.wait(timeout=timeout)
        except subprocess.TimeoutExpired:
            os.killpg(process.pid, signal.SIGKILL)
            process.wait()
            return 124


def snapshot(repo, revision, out):
    """Export committed inputs only; neither dirty source nor binaries enter S."""
    if out.exists():
        raise ValueError(f"bundle destination already exists: {out}")
    commit = capture(["git", "-C", str(repo), "rev-parse", revision + "^{commit}"]).decode().strip()
    archive = capture(["git", "-C", str(repo), "archive", commit])
    out.mkdir(parents=True)
    with tarfile.open(fileobj=io.BytesIO(archive)) as tar:
        # Git trees may contain relative symlinks; reject links escaping the bundle.
        for member in tar.getmembers():
            destination = out / member.name
            if not destination.resolve().is_relative_to(out.resolve()):
                raise ValueError("archive path escapes bundle")
            if member.issym() and not (destination.parent / member.linkname).resolve().is_relative_to(out.resolve()):
                raise ValueError("archive link escapes bundle")
        tar.extractall(out, filter="data")
    files = {p.relative_to(out).as_posix(): sha(p) for p in sorted(out.rglob("*")) if p.is_file()}
    links = {p.relative_to(out).as_posix(): os.readlink(p) for p in sorted(out.rglob("*")) if p.is_symlink()}
    write_json(out / "reference-bundle.json", {"commit": commit, "files": files, "links": links})
    print(f"reference bundle {commit}: {len(files)} files in {out}")


def verify_bundle(bundle):
    manifest = json.loads((bundle / "reference-bundle.json").read_text())
    for name, target in manifest.get("links", {}).items():
        if not (bundle / name).is_symlink() or os.readlink(bundle / name) != target:
            raise ValueError(f"reference bundle link changed: {name}")
    for name, wanted in manifest["files"].items():
        path = bundle / name
        if not path.is_file() or sha(path) != wanted:
            raise ValueError(f"reference bundle changed: {name}")
    # Additional implementation files must not silently join a source list.
    present = {p.relative_to(bundle).as_posix() for p in (bundle / "dev").rglob("*") if p.is_file()}
    expected = {p for p in manifest["files"] if p.startswith("dev/")}
    if present != expected:
        raise ValueError("reference bundle dev inventory changed")
    return manifest


def inventory(directory):
    return {p.relative_to(directory).as_posix(): sha(p) for p in sorted(directory.rglob("*.o"))}


def binaries(directory):
    return {tool: sha(directory / (tool + "-self")) for tool in TOOLS
            if (directory / (tool + "-self")).is_file()}


def same(expected, actual, exact=True):
    return (actual["exit"] == 0 and bool(expected["objects"])
            and set(actual["objects"]) == set(expected["objects"])
            and set(actual["binaries"]) == set(expected["binaries"]) == set(TOOLS)
            and (not exact or (actual["objects"] == expected["objects"]
                               and actual["binaries"] == expected["binaries"])))


def dependencies(directory):
    """Record actual host headers as well as the versioned source inputs."""
    paths = set()
    for depfile in directory.rglob("*.d"):
        if not depfile.is_file():
            continue
        text = depfile.read_text().replace("\\\n", " ")
        for line in text.splitlines():
            if ":" not in line:
                continue
            for name in line.split(":", 1)[1].split():
                path = Path(name)
                if path.is_absolute() and path.is_file():
                    paths.add(path)
    return {str(p): sha(p) for p in sorted(paths)}


class Bootstrap:
    def __init__(self, bundle, out, host, jobs=8):
        self.bundle, self.out = bundle.resolve(), out.resolve()
        self.manifest = verify_bundle(self.bundle)
        self.host = Path(shutil.which(str(host)) or host).resolve(strict=True)
        self.jobs = jobs
        if jobs < 1 or jobs > 8:
            raise ValueError("reference build jobs must be between 1 and 8")
        self.work = self.out / "work"
        self.objects = self.work / "obj/pa34/selfhost"
        self.bins = self.work / "obj/pa34/bin/selfhost"
        self.saved = self.out / "generations"
        self.env = environment(jobs)
        # Include harness code, host/linker/runtime identity and canonical paths.
        versions = capture([str(self.host), "-v"]).decode()
        system = {"host": sha(self.host), "version": versions,
                  "machine": os.uname().machine}
        # -MMD excludes system headers. Hash the host C++ header closure too.
        headers = capture([str(self.host), "-std=gnu++11", "-M", "-x", "c++",
                           "-include", "bits/stdc++.h", "/dev/null"]).decode().replace("\\\n", " ")
        system["headers"] = {p: sha(p) for p in headers.split(":", 1)[1].split()
                             if Path(p).is_absolute() and Path(p).is_file()}
        for item in ("libstdc++.so", "libgcc_s.so.1", "libc.so.6"):
            path = Path(capture([str(self.host), "-print-file-name=" + item]).decode().strip())
            if path.is_file():
                system[item] = sha(path)
        linker = Path(capture([str(self.host), "-print-prog-name=ld"]).decode().strip())
        linker = Path(shutil.which(str(linker)) or linker)
        system["linker"] = sha(linker)
        self.inputs = {"protocol": PROTOCOL, "script": sha(__file__), "bundle": digest(self.manifest),
                       "host": str(self.host), "system": system, "work": str(self.work),
                       "flags": "gnu++11,-O3,TEST_RUNNER_ENABLE,host-link,default-allocator"}

    @contextlib.contextmanager
    def locked(self):
        self.out.mkdir(parents=True, exist_ok=True)
        with (self.out / "lock").open("w") as lock:
            fcntl.flock(lock, fcntl.LOCK_EX)
            self.prepare()
            yield

    def prepare(self):
        identity = self.out / "bundle.json"
        if not (self.work / ".prepared").exists() or (identity.exists() and json.loads(identity.read_text()) != self.inputs):
            # Never combine source sets or changed recipes at canonical work paths.
            shutil.rmtree(self.work, ignore_errors=True)
        write_json(identity, self.inputs)
        (self.work / "pa34").mkdir(parents=True, exist_ok=True)
        # Some debug prerequisites probe ../dev tools even with explicit APP
        # overrides. Keep mutable tool aliases away from the frozen source.
        if not (self.work / "dev").exists():
            shutil.copytree(self.bundle / "dev", self.work / "dev", symlinks=True)
        for name in ("scripts", "shared"):
            target = self.work / name
            if not target.exists():
                target.symlink_to(self.bundle / name, target_is_directory=True)
        shutil.copyfile(self.bundle / "pa34/Makefile", self.work / "pa34/Makefile")
        for number in range(1, 34):
            target = self.work / f"pa{number}"
            if not target.exists():
                shutil.copytree(self.bundle / f"pa{number}", target, symlinks=True)
        self.saved.mkdir(parents=True, exist_ok=True)
        (self.work / ".prepared").touch()

    def recipe(self, compiler):
        return ["make", "--no-print-directory", "-k", "-j1", "-C", str(self.work / "pa34"), "all",
                "CXX=" + str(compiler), "CPPGM_HOST_CXX=" + str(self.host),
                "INCEPTION_COMPILER_FLAVOR=selfhost", "INCEPTION_COMPARE_OBJECTS=0",
                "INCEPTION_BUILD_MISSING_FIRST=0", f"INCEPTION_BUILD_JOBS={self.jobs}",
                f"INCEPTION_OBJECT_BUILD_JOBS={self.jobs}", "INCEPTION_SELFHOST_OPT_LEVEL=3",
                "INCEPTION_OBJ_ROOT_BASE=../obj/pa34", "CPPGM_TEST_RUNNER=1"]

    def verify_work(self):
        expected = {p: h for p, h in self.manifest["files"].items() if p.startswith("dev/")}
        for name, wanted in expected.items():
            if not (self.work / name).is_file() or sha(self.work / name) != wanted:
                raise ValueError(f"working reference source changed: {name}")
        present = {p.relative_to(self.work).as_posix() for p in (self.work / "dev").rglob("*") if p.is_file()}
        allowed = set(expected) | set(self.manifest.get("links", {})) | {"dev/" + t for t in TOOLS}
        if present - allowed:
            raise ValueError("working reference source inventory changed")
        if sha(self.work / "pa34/Makefile") != sha(self.bundle / "pa34/Makefile"):
            raise ValueError("working reference recipe changed")

    def valid(self, path, inputs):
        try:
            result = json.loads((path / "receipt.json").read_text())
            return (result["inputs"] == inputs and result["exit"] == 0
                    and bool(result["objects"]) and set(result["binaries"]) == set(TOOLS)
                    and result["objects"] == inventory(path / "objects")
                    and result["binaries"] == binaries(path / "bin")
                    and all(Path(p).is_file() and sha(p) == h for p, h in result["dependencies"].items()))
        except (OSError, ValueError, KeyError):
            return False

    def generation(self, name, compiler, fresh=False):
        self.verify_work()
        compiler = Path(compiler).resolve(strict=True)
        inputs = dict(self.inputs, compiler=str(compiler), compiler_sha256=sha(compiler))
        index = self.out / f"cache-{name}-{digest(inputs)}.json"
        if not fresh and index.exists():
            saved = Path(json.loads(index.read_text())["path"])
            if self.valid(saved, inputs):
                print(f"reference bootstrap: reuse {name} {saved.name}", flush=True)
                return json.loads((saved / "receipt.json").read_text())
        shutil.rmtree(self.objects, ignore_errors=True)
        shutil.rmtree(self.bins, ignore_errors=True)
        code = execute(self.recipe(compiler), self.work, self.out / f"{name}.log", self.env)
        verify_bundle(self.bundle)
        if sha(compiler) != inputs["compiler_sha256"]:
            raise ValueError("compiler changed during reference build")
        result = {"inputs": inputs, "exit": code, "objects": inventory(self.objects),
                  "binaries": binaries(self.bins), "dependencies": dependencies(self.objects)}
        saved = self.saved / (name + "-" + digest(result))
        if saved.exists() and (inventory(saved / "objects") != result["objects"]
                               or binaries(saved / "bin") != result["binaries"]):
            shutil.rmtree(saved)
        if not saved.exists():
            saved.mkdir()
            if self.objects.exists():
                shutil.copytree(self.objects, saved / "objects")
            if self.bins.exists():
                shutil.copytree(self.bins, saved / "bin")
        result["path"] = str(saved)
        write_json(saved / "receipt.json", result)
        if code == 0 and bool(result["objects"]) and set(result["binaries"]) == set(TOOLS):
            write_json(index, {"path": str(saved)})
        return result

    def pair(self, compiler, fresh=False):
        baseline = self.generation("B", self.host, fresh)
        if baseline["exit"] or set(baseline["binaries"]) != set(TOOLS):
            raise ValueError("host reference build failed; see B.log")
        candidate = self.generation("A", compiler, fresh)
        complete = same(baseline, candidate, exact=False)
        result = {"complete": complete, "bundle": digest(self.manifest),
                  "compiler": str(Path(compiler).resolve()), "compiler_sha256": sha(compiler),
                  "baseline": str(Path(baseline["path"]) / "bin/cppgm++-self"),
                  "candidate": str(Path(candidate["path"]) / "bin/cppgm++-self"),
                  "baseline_receipt": baseline, "candidate_receipt": candidate}
        write_json(self.out / "pair.json", result)
        return result

    def suites(self, generation):
        self.verify_work()
        directory = Path(generation["path"]) / "bin"
        passed = run_suites(self.work, directory, self.host, self.jobs,
                            self.out / "suites" / Path(generation["path"]).name)
        verify_bundle(self.bundle)
        return passed

    def final(self, compiler, profile):
        write_json(self.out / "proof.json", {"profile": profile, "passed": False,
                                            "compiler_sha256": sha(compiler), "status": "running"})
        pair = self.pair(compiler, fresh=True)
        if not pair["complete"]:
            return False
        first = pair["candidate_receipt"]
        second = self.generation("C", pair["candidate"], fresh=True)
        if second["exit"]:
            return False
        third = self.generation("D", Path(second["path"]) / "bin/cppgm++-self", fresh=True)
        stable = same(second, third)
        if profile == "port":
            reference = self.generation("F", pair["baseline"], fresh=True)
            stable = stable and same(reference, first) and same(first, second)
        suites = stable and self.suites(first) and self.suites(third)
        write_json(self.out / "proof.json", {"profile": profile, "pair": pair, "C": second,
                                            "D": third, "stable": stable, "suites": suites,
                                            "passed": stable and suites})
        print("reference-bootstrap/fixed-point: " + ("PASS (1/1)" if stable else "FAIL (0/1)"))
        print("reference-bootstrap/rebuilt-suites: " + ("PASS (1/1)" if suites else "FAIL (0/1)"))
        return stable and suites


def bindings(directory, stage, host, rust=False):
    primary = {1: "pptoken", 2: "posttoken", 3: "ppexpr", 4: "preproc", 8: "lowir",
               9: "abimangle", 24: "lowir2native", 32: "lowiropt", 33: "lowir2native"}.get(stage, "cppgm++")
    binary = lambda tool: str(directory / (("cppgm" if tool == "cppgm++" else tool) if rust else tool + "-self"))
    values = {"CPPGM_SKIP_DEV_REBUILD": "1", "TEST_DEPS": "", "RUN_CHECK_DEPS": "",
              "CPPGM_TEST_RUNNER": "0" if rust else "1", "CXX": str(host), "CPPGM_HOST_CXX": str(host),
              "CPPGM_TEST_APP": binary(primary), "CPPGM_DRIVER_APP": binary("cppgm++"),
              "LOWIROPT_APP": binary("lowiropt"), "LOWIR2NATIVE_APP": binary("lowir2native"),
              "NATIVE_REFERENCE_APP": binary("lowir2native"),
              "CPPGM_LOWIR_BEHAVIOR_APP": binary("lowir2native"),
              "CPPGM_CPPGM_APP": binary("cppgm++"), "CPPGM_LOWIROPT_APP": binary("lowiropt"),
              "CPPGM_LOWIR2NATIVE_APP": binary("lowir2native")}
    return [f"{key}={value}" for key, value in values.items()]


def run_suites(root, directory, host, jobs, out, rust=False):
    if not rust:
        for tool in TOOLS:
            target = root / "dev" / tool
            target.unlink(missing_ok=True)
            target.symlink_to(directory / (tool + "-self"))
    passed_total = total = 0
    for stage, target in [(n, "test") for n in range(1, 34)] + [(n, "test-debuginfo") for n in (8, 32, 33)]:
        cwd = root / f"pa{stage}"
        marker = cwd / ".test_failed"
        marker.unlink(missing_ok=True)
        logdir = out / f"pa{stage}" / target
        logdir.mkdir(parents=True, exist_ok=True)
        counts = logdir / "counts.txt"
        counts.unlink(missing_ok=True)
        env = dict(environment(jobs), CPPGM_TEST_COUNTS_FILE=str(counts))
        command = ["make", "--no-print-directory", "-s", target, *bindings(directory, stage, host, rust)]
        code = execute(command, cwd, logdir / "course.log", env, timeout=3600)
        rows = [line.split() for line in counts.read_text().splitlines()] if counts.exists() else []
        valid = bool(rows) and all(len(row) == 2 and all(v.isdecimal() for v in row) for row in rows)
        counted = valid and all(int(row[0]) == int(row[1]) for row in rows)
        passed = code == 0 and not marker.exists() and counted
        if valid:
            passed_total += sum(int(row[0]) for row in rows)
            total += sum(int(row[1]) for row in rows)
        write_json(logdir / "result.json", {"exit": code, "counts": rows, "passed": passed})
        if not passed:
            print(f"reference-course/pa{stage}/{target}: FAIL (0/1); see {logdir}/course.log")
            return False
    print(f"===== ALL TESTS PASSED SUCCESSFULLY! ({passed_total}/{total}) =====")
    return True


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action", choices=("snapshot", "identity", "pair", "progress", "final", "course"))
    parser.add_argument("--repo", type=Path)
    parser.add_argument("--revision", default="HEAD")
    parser.add_argument("--bundle", type=Path)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--compiler", type=Path)
    parser.add_argument("--course-root", type=Path)
    parser.add_argument("--tools-dir", type=Path)
    parser.add_argument("--host", default="g++")
    parser.add_argument("--jobs", type=int, default=8)
    parser.add_argument("--profile", choices=("student", "port"), default="student")
    parser.add_argument("--fresh", action="store_true")
    args = parser.parse_args()
    if args.action == "course":
        if not args.course_root or not args.tools_dir:
            parser.error("course requires --course-root and --tools-dir (Rust release binaries)")
        return 0 if run_suites(args.course_root.resolve(), args.tools_dir.resolve(), args.host,
                              args.jobs, args.out.resolve(), rust=True) else 1
    if args.action == "snapshot":
        if not args.repo:
            parser.error("snapshot requires --repo")
        snapshot(args.repo, args.revision, args.out.resolve())
        return 0
    if not args.bundle or not args.compiler:
        parser.error("build actions require --bundle and --compiler")
    with args.compiler.open("rb") as executable:
        if executable.read(4) != b"\x7fELF":
            parser.error("--compiler must be the actual Linux release executable, not a build wrapper")
    boot = Bootstrap(args.bundle, args.out, args.host, args.jobs)
    if args.action == "identity":
        print(digest(dict(boot.inputs, compiler=str(args.compiler.resolve()), compiler_sha256=sha(args.compiler))))
        return 0
    with boot.locked():
        if args.action == "final":
            return 0 if boot.final(args.compiler, args.profile) else 1
        pair = boot.pair(args.compiler, args.fresh)
        if args.action == "progress":
            expected, actual = pair["baseline_receipt"], pair["candidate_receipt"]
            if args.profile == "port":
                expected = boot.generation("F", pair["baseline"])
            complete = same(expected, actual, exact=args.profile == "port")
            print("===== pa34 =====")
            for category in ("objects", "binaries"):
                for name, wanted in expected[category].items():
                    ok = name in actual[category] and (args.profile != "port" or actual[category][name] == wanted)
                    print(f"reference-bootstrap/{category}/{name}: " + ("PASS (1/1)" if ok else "FAIL (0/1)"))
            print("reference-bootstrap/complete-inventory: " + ("PASS (1/1)" if complete else "FAIL (0/1)"))
            return 0 if complete else 1
        return 0 if pair["complete"] else 1


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        print(f"reference bootstrap infrastructure error: {error}", file=sys.stderr)
        sys.exit(2)

# Public benchmarks

The benchmarks build third-party programs with `cppgm++`, `g++` and `clang++`
and compare the results. The **Public benchmarks** workflow runs on every push.

## Files

- `manifest.json`: the sources, their pinned commits and the programs to build.
- `patches/`: the changes to the sources.
- `drivers/`: `main` functions for libraries.
- `baseline.json`: the recorded instruction counts.

## Run

```sh
python3 scripts/public_benchmarks.py fetch --cache obj/public-benchmarks/src
python3 scripts/public_benchmarks.py run --cache obj/public-benchmarks/src \
  --work obj/public-benchmarks/work --output obj/public-benchmarks/results.json
python3 scripts/public_benchmarks.py report obj/public-benchmarks/results.json
```

Options for `run`:

- `--only REGEX`: run the programs that match.
- `--no-count`: do not run `cachegrind`.
- `--no-clang`: do not run `clang++`.
- `--timing-blocks N`: paired blocks for a short program (default: 2).
  `0` turns off the pass. `--timing-rounds` remains an alias.

## Measurements

All programs, at `-O0` and `-O3`:

- Compile CPU time and peak memory.
- Executable size.
- Run CPU time.

All programs, once: frontend CPU time, from `-fsyntax-only`.

The `-O3` timing pass reuses the built executables, one program at a time.
For each reference compiler, A is its executable and B is `cppgm++`'s.
The pass alternates four-run ABBA and BAAB blocks and takes the median of
the blocks' CPU ratios; each block uses the mean of its two runs per side.
Programs whose initial `g++` run takes at least 1 second get one block;
shorter programs get two. A four-run GCC A/A calibration records timing
variation. Every observation is retained, including failures; each timed
run must reproduce its original stdout and exit code.

Programs with `count` in the manifest, at `-O3`, from
`valgrind --tool=cachegrind`:

- `compile`: `cppgm++` instructions.
- `run`: instructions of the `cppgm++` and `g++` binaries.

`count` applies only to single-threaded programs with repeatable counts.

`g++` links the objects from all three compilers. The references use stock
flags. To turn off their vectorizers, set `references` in the manifest:

```json
"references": {
 "gcc": ["-fno-tree-vectorize"],
 "clang": ["-fno-vectorize", "-fno-slp-vectorize"]
}
```

## Failures

A run fails if:

- `cppgm++` cannot build a program that `g++` builds.
- The output or exit code of a `cppgm++` binary differs from `g++` at the same
  level.

A `clang++` failure does not fail the run. The program is left out of the
`clang++` column.

Differences in the numbers do not fail the run. `report` writes a warning when
a `cppgm++` count differs from `baseline.json`:

- Compile counts: more than 0.5%.
- Run counts: more than 1%.

## Reading the README table

The [EPYC report](https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/epyc/report.md),
[dedicated Intel report](https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/intel/report.md)
and [GitHub Intel report](https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/github-intel/report.md)
list every program's O0, O3 and frontend compile CPU times for all three
compilers, with higher-is-better speed ratios. They include compiler rebuild
times, paired self-build blocks, toolchains and links to the exact CI run and
raw measurements. The same tables appear in each new workflow run's summary
and its `details.md` artifact.

Program speed is `GCC CPU time / cppgm++ CPU time`: 1× is equal, and higher
is faster. Compile speed measures compilation, excluding linking;
generated code speed measures execution of the resulting programs. The `-O3`
execution timings use the separate timing pass described above.

For each of these four program rows, the value is the geometric mean of the
per-program ratios, giving each included program equal weight. Only passing
programs with both measurements are included; CPU times below 0.01 seconds on
either side are excluded. The `-O3` timed programs row shows how many programs
contributed to that generated-code ratio. Cutoffs apply separately on each host,
so counts can differ between columns.

The two compiler build rows time the same compiler sources at `-O3`, with the
same job count:

- **Building `cppgm++`**: GCC's build time divided by the time taken by the
  GCC-built `cppgm++` to build itself.
- **Self-built compiler speed**: that self-build time divided by the time
  taken by the self-built compiler to compile the same sources. After the
  initial bootstrap, fixed GCC-built and self-built binaries rebuild identical
  PA34 inputs in two ABBA/BAAB blocks, following warmups and four GCC-built A/A runs.
  The badge uses the median paired CPU ratio. Every rebuild must produce
  identical objects and compiler binaries.

`selfhost --paired-blocks N` changes the rebuild block count. The initial GCC
build is measured once for the **Building `cppgm++`** row. Pairing reduces
order effects; A/A variation still limits claims about small differences.
The results JSON and workflow artifacts retain the calibration, block ratios,
raw observations and output hashes.

EPYC and Intel columns retain their latest separate measurements, which can be
from different commits. The measured commit row and the provenance links below
identify the revision, CPU, toolchains and workflow for each column. The
publisher renders compact, rounded SVG badges. A failed program or self-hosted
build makes the affected speed badge show the failure count. The job summary
also reports Clang comparisons.

## Update the baseline

1. Download `results.json` from the CI artifact.
2. Run:

   ```sh
   python3 scripts/public_benchmarks.py report results.json \
     --write-baseline benchmarks/public/baseline.json
   ```

3. Commit `baseline.json` with the change that moved the counts.

Do not record the baseline on a local machine.

## Add a program

1. Add the source and its pinned commit to `manifest.json`.
2. Add the program to `manifest.json`.
3. Make sure that `g++` and `clang++` accept the program.

Patch a source only if the code is ill-formed. Do not patch valid code. Fix the
compiler. The current patches change:

- String literals assigned to `char *`.
- Narrowing in braces.
- Surrogate universal character names.
- A missing `typename` in `tramp3d-v4`.
- An assignment to a `const` member in `rapidjson`.
- A Latin-1 byte in `hexxagon`.
- x87 inline assembly in `minisat`.

## Optional Intel CI run

The Public benchmarks workflow runs on GitHub's Linux pool by default, which
can supply EPYC or Intel CPUs. Automatic runs publish to `epyc` or `github-intel`
according to the measured CPU. The dedicated Intel runner uses `intel`;
publishing any host preserves the other hosts and the legacy EPYC URLs.
[EPYC provenance](https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/epyc/provenance.json)
and [Intel provenance](https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/intel/provenance.json) record the exact CPU,
compiler commit, toolchains and workflow URL. The README shows the measured
commit because the hosts' latest measurements can be from different commits.

On the Intel Linux host, with `gh` authenticated and Podman or Docker installed:

```sh
python3 scripts/run_intel_benchmarks_once.py --ref main --revision main
```

This downloads the official GitHub runner, verifies its SHA-256 digest,
registers it for one job, dispatches Public benchmarks with `platform=intel`,
and removes its registration after the job. It keeps logs and measurements
in the printed temporary directory, removes the runner credentials, and
installs no service. Choose only a trusted compiler revision.

To trigger from the Actions UI instead, start the helper with `--listen`,
then choose Public benchmarks → Run workflow → `platform=intel`. Keep
`runner_label=cppgm-intel` and select the compiler revision to measure. An
Intel job waits for this temporary runner; ordinary pushes never queue it.
The Intel job uses Ubuntu 26.04 and the same benchmark, GCC, Clang, serial
timing and self-host measurements as the automatic run. Run it while the
host is otherwise idle.

For the optional course exercise measurements, enable `course_quality` in
either manual CPU lane, or add `--course-quality` to the Intel helper. This
checks the scalar oracles, records Cachegrind work, and retains A/A calibration
and six paired timing blocks under `course-quality/` in the results artifact.
Use the same compiler revision in both lanes. These diagnostic timings do not
change the badges or impose a hardware-specific course gate.

## index-cpp comparison

The **index-cpp benchmarks** workflow runs on every push and pull request.
It builds the unchanged, pinned upstream C++17 program with cppgm, GCC and
Clang and requires all 50 checksum workloads to pass. This reference-only
lane does not add C++17 requirements to student assignments.

[index-cpp reports by host](https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/index-cpp/report.md)
show each workload, higher-is-better GCC and Clang speed ratios, compile
cost, code size and A/A timing warnings. Its aggregate is separate from the
public-suite badges. Main runs retain comparisons and raw observation JSON
beside the existing results on `benchmark-badges`, with commit history under
`index-cpp/history/`; all runs also attach full receipts to their CI summary.

```sh
python3 scripts/index_cpp_ci.py run --output obj/index-cpp
```

The runner uses full upstream inputs, three alternating ABBA/BAAB blocks per
reference and one A/A block for runtime and compilation. It pins the source
commit and hash, checks repeated compile outputs, and records CPU affinity,
toolchains, binary hashes and header inventories. Hardware counters are used
when permitted; an unavailable-counter reason remains in the report. A/A
variation above 2% warns without omitting slow or noisy workloads. Checksums
and build failures fail CI; timing changes remain measurements for review.

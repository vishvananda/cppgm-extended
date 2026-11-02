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
- `--timing-rounds N`: the runs of a short program in the timing pass. The
  default is 5. `0` turns off the pass.

## Measurements

All programs, at `-O0` and `-O3`:

- Compile CPU time and peak memory.
- Executable size.
- Run CPU time.

All programs, once: frontend CPU time, from `-fsyntax-only`.

The `-O3` run CPU time comes from a timing pass. The pass runs after all
builds, one program at a time. Each round runs the binary of each compiler
once. A program whose `g++` binary runs for less than 1 second gets 5 rounds,
and other programs get 1 round. The result is the least CPU time of the
rounds.

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

## Badges

The root README has a table of six badges. Each badge gives `cppgm++` speed as
a multiple of `g++` speed. Higher is better.

| Row | Column | Label | Measure |
| --- | --- | --- | --- |
| `-O0` | compile speed | `-O0 compile speed vs gcc` | geometric mean of compile CPU time at `-O0` |
| `-O0` | generated code speed | `-O0 generated speed vs gcc` | geometric mean of run CPU time at `-O0` |
| `-O3` | compile speed | `-O3 compile speed vs gcc` | geometric mean of compile CPU time at `-O3` |
| `-O3` | generated code speed | `-O3 generated speed vs gcc` | geometric mean of run CPU time at `-O3` |
| Building `cppgm++` | compile speed | `self compile speed vs gcc` | CPU time of the `-O3` build of `cppgm++` by the `g++`-built `cppgm++`, against the same build by `g++` |
| Building `cppgm++` | generated code speed | `self-built speed vs gcc` | CPU time of the `-O3` build of `cppgm++` by the self-built `cppgm++`, against the same build by the `g++`-built `cppgm++` |

The badge JSON holds the label. The table hides it with `&label=` in the URL.

The three builds use the same job count.

The job summary uses the same format. It gives the reference over `cppgm++` for
each measure, so above 1× means `cppgm++` is smaller or faster.

Each push to `main` writes the badges to the `benchmark-badges` branch. If a
program or a self-hosted build fails, the badges it affects are red and give the
number of failures. `clang++` results are in the job summary.

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

The Public benchmarks workflow runs on GitHub's Linux pool by default. Its
EPYC badges and the optional Intel badges occupy separate directories on
`benchmark-badges`; publishing either keeps the other column's results.
[EPYC provenance](https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/epyc/provenance.json)
and [Intel provenance](https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/intel/provenance.json) record the exact CPU,
compiler commit, toolchains and workflow URL. The README shows the measured
commit because a manual Intel run can be older than the automatic EPYC run.

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

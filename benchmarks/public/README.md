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

## Measurements

All programs, at `-O0` and `-O3`:

- Compile CPU time and peak memory.
- Executable size.
- Run CPU time.

All programs, once: frontend CPU time, from `-fsyntax-only`.

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

The root README has one badge for `-O0` and one for `-O3`. Each badge gives
`cppgm++` speed as a multiple of `g++` speed. Higher is better.

- Compile speed: the geometric mean of compile CPU time.
- Generated speed: the geometric mean of run CPU time.

Each push to `main` writes the badges to the `benchmark-badges` branch. If a
program fails, the badge is red and gives the number of failures. `clang++`
results are in the job summary.

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

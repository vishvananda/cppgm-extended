Bootstrap/self-host reducers and preserved reductions for the current tree.

These are not assignment regressions. They are checked-in reduction artifacts used
to preserve active self-host compile blockers between checkpoints.

Run a reducer with:

```sh
make run-cppgm CPPGM_ARGS='-c -I dev/src -o /tmp/reducer.o <path-to-reducer>'
```

Current open reducers:

- `bsc-open-assign-value-lambda.cpp`
  - reproduces the `unknown function assign_value` class-template/lambda lookup failure

Preserved fixed reductions:

- `bsc-open-map-string-index-move.cpp`
  - previously reproduced the hosted `std::map<std::string, ...>::operator[]` move-path crash
  - now passes on the current tree and is kept as a reduced artifact for this checkpoint
- `bsc-open-function-partial-copy-assign.cpp`
  - previously reproduced failure to apply a namespace-qualified function-type partial
    specialization's out-of-class copy-assignment definition
  - now passes on the current tree and is kept as a reduced artifact for this checkpoint
- `bsc-open-lambda-reference-capture-shadow.cpp`
  - previously reproduced a missed implicit lambda capture hidden in a
    declaration-shaped function-style local initializer
  - now passes on the current tree and is kept as a reduced artifact for this checkpoint
- `bsc-open-alias-pack-expansion-return.cpp`
  - previously reproduced failure to expand a deduced type pack nested inside
    dependent alias-template arguments in a function-template return type
  - now passes on the current tree and is kept as a reduced artifact for this checkpoint
- `bsc-open-class-template-init-list-ctor.cpp`
  - previously reproduced failure to copy an out-of-class class-template constructor
    definition when a structured class-template argument still depended on the owner
    template parameter
  - now passes on the current tree and is kept as a reduced artifact for this checkpoint

Keep the owning status and any broader frontier notes in
`legacy/BOOTSTRAP_SELFHOST_FRONTIER_TRACKER.md`.

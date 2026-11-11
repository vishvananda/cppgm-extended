# Benchmark publication storage

`benchmark-badges` serves SVG badges, host/compiler provenance, shared comparison
records and readable result tables. README links and host-specific paths remain
stable. Full logs, XML, commands, executable receipts and individual raw timing
observations belong to the linked Actions run's artifacts. Their retention is
set by each workflow; download an artifact before it expires when longer-term
raw evidence is needed.

Boost publishes its per-test timing tables and the shared version-1 comparison
schema at `results.json` and `standard/records.json`. Its original campaign
document is available as `results.json` inside the CI artifact. Shared records
link to one host metadata file, with its canonical JSON hash, rather than
embedding that same metadata in every comparison. The measurements and their
methods are unchanged.

Every publisher runs `scripts/benchmark_publication.py prepare` before staging
files. This canonicalizes JSON and checks the allowlist, a 2 MiB per-file limit
and a 64 MiB total limit. Raw evidence is rejected even when it is small.
Historical report directories retain their measured revision and CPU.

## Removing the previous bulk data

Prepare a compact copy without materializing the raw logs:

```sh
python3 scripts/benchmark_publication.py compact-git . origin/benchmark-badges /tmp/compact-badges
python3 scripts/benchmark_publication.py check /tmp/compact-badges
```

The command writes a new directory and changes no refs. It preserves the SVGs,
tables and provenance, replaces historical Boost campaign documents with their
existing comparison records, and directs raw-evidence links to their CI runs.
Expired artifacts cannot be recovered from those runs.

Deleting files in an ordinary commit leaves their old blobs in Git history.
Reclaiming clone size therefore requires a one-time replacement of the generated
branch with the reviewed compact tree. Save the old ref in a local Git bundle,
coordinate with in-flight publishers, and use a lease against the exact reviewed
remote tip. Do not rewrite the compiler's main branch or its tags. GitHub's
repository-size counter may change only after server-side garbage collection.

The owner can run **Compact benchmark publication** on main after older
benchmark campaigns finish or are cancelled. It uses the publishers' concurrency
group, retains a rollback bundle as a 30-day Actions artifact, and replaces the
generated branch with a lease against the tip it actually reviewed.

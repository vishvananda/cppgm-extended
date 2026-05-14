# Async Small-Fix Discovery Notes

This directory is the results area for `ASYNC_SMALL_FIX_DISCOVERY_PROCESS.md`.

It intentionally replaces the idea of a shared tracker for this workflow.

Rules:

- one note file per worktree investigation
- use a unique filename, typically `YYYY-MM-DD-<slug>.md`
- write the note in the main checkout, not inside the tmp worktree
- keep each note standalone and self-contained
- record the `obj/` seed source, or explicitly say the worktree was built cold
- record either a committed closure or an explicit abandon reason
- do not commit these notes from the main checkout; they are coordination records for parallel
  investigations
- do not edit other investigations just to keep a central queue in sync

The point is to let several async small-fix branches proceed independently and merge later without
everyone touching the same tracker file.

These notes are candidate imports for the serial hosted/bootstrap processes, not automatic merge
authorizations. Before a note's closure commit is pulled into the active branch, revalidate that
commit on current `HEAD`. Prefer staged validation in the warm main checkout by applying the
closure, staging only the imported files, and stashing unrelated live work with
`git stash push --keep-index -u`, then rerun the note's direct repro, the required `dev/` build
target(s), and the normal `verify-fast-pa10-31-nobuild` command. Fall back to the persistent
clean async-intake worktree only when overlap or stash/index issues make the staged path unsafe.
Only rerun a separate owning suite when it is needed for faster iteration or when the owner is
outside that gate.

For fresh async branches, prefer seeding `obj/` from a clean same-`HEAD` seed worktree with
`scripts/seed_async_worktree_obj.sh` before the first narrow `make -C dev ...`.
That helper also refreshes the copied object mtimes so a new checkout does not force a near-cold
rebuild immediately.

Start new notes from `TEMPLATE.md`.

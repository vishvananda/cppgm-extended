# Approaching machine optimization

Start with the MIR and native backend you built in PA24. The
[assignment](README.md) defines the required behavior and code-quality bounds;
these suggestions describe one route to them.

First establish which values and physical registers each instruction reads,
writes or clobbers. Include call arguments, fixed-register operations, both
parts of indexed addresses, and values live across control-flow edges.
Incorrect use information makes even a redundant-copy removal unsafe.

For O1, small local rewrites are useful: remove identity copies, forward a
value into its next use, fold a cheap address or immediate, and eliminate a
jump to the next block. Stop propagation at a conflicting definition or
clobber. Preserve volatile effects, exception boundaries and debug locations
when replacing instructions.

For O2, compute liveness across blocks and calls. Use it to avoid unnecessary
spills and reloads, retain frequently used loop values, and reuse stack slots
whose lifetimes do not overlap. Linear scan, graph coloring or another bounded
allocator can all work. Make conservative placement the fallback when a proof
or work budget is unavailable. Recompute final frame and callee-saved metadata
from the instructions you will encode.

Keep each improvement small enough to validate with an existing fixture.
Compare your own O0 and optimized dumps to understand the redundant work, then
read the failing expectation to see the required bound. Reference dumps are
examples of one successful answer. They are not pass recipes.

For performance investigation beyond the assignment, use programs that run
long enough to measure and depend on runtime inputs. Keep their results equal,
build outside the timed region, and compare immutable executable versions in
wall-time ABBA order. Repeat A/A runs to estimate noise before interpreting a
small difference. Such measurements help choose profitable changes; the
assignment's completion checks remain deterministic.

# Assignment Numbering Migration — 2026-08

The standalone typed Itanium ABI encoder moved ahead of the first
source-to-LowIR assignment so every compiler-emitted symbol can use the shared
encoder from its first appearance.

| Before | Current | Assignment |
| --- | --- | --- |
| PA30 | PA14 | `abimangle` typed ABI facts and name construction |
| PA14-PA27 | PA15-PA28 | source-to-LowIR language and object-model milestones |
| PA28 | PA29 | `lowir2native` backend |
| PA29 | PA30 | `cppgm++` separate-compilation and compile/link driver |

PA1-PA13 and PA31-PA39 retain their numbers.

Existing assignment tests, references, and course placeholders moved with
their owning assignment. Live build rules, export manifests, placement policy,
performance inputs, and PA39 checkpoints use the current numbers.

Historical investigation logs and completed intake trackers may retain the
number that was current when an event was recorded. Interpret those entries
through the table above; do not treat an old path in a historical record as a
current feature owner.

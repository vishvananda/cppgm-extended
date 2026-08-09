# Witness convergence reset evidence

This directory preserves the exact strict-failure set comparison used by
`docs/witness-convergence-reset-and-recovery-plan.md` on 2026-08-08.

- Clean control: `5add5290c69be6b76138dfc1f6696915eb0278ae`
- Dirty compiler state: the archive side reference recorded by recovery Phase 0
- Strict corpus: 1,530 patched-Clang `.ref.witness` files
- Clean failures: 191
- Dirty failures: 312
- Fail on both: 173 (`common-failures.txt`)
- Fixed by dirty state: 18 (`dirty-fixed-failures.txt`)
- Regressed only in dirty state: 139 (`dirty-only-failures.txt`)

Ephemeral audit artifacts at the time of capture were:

| Artifact | SHA-256 |
| --- | --- |
| Dirty mismatch analysis JSON | `6e6c02acd42b1ff747c451d0873102de1459bfac18683136edf5b54dcbf5e14d` |
| Dirty strict log | `75d006783973ce130f72fbdd8863b8b906f7d026512bb7e8f989c911b0659eea` |
| Clean strict log | `1c110e51e9f2d23115e0d6eca98b55e8b58a0d279c1f97ce2b7f8dc0d48f98cf` |
| Dirty broad report | `e9f1978a450b710ed57fc009766a66310d5e314d6246dc0995315a4a39e748b5` |
| First performance check | `645291a82bcdae84e80ef8ddd138c9a1ab060b0adf87956734bb9cc10ea9b302` |
| RSS confirmation check | `50655ba49dc3b9d11ec49fda4034f2aa990ebc42f387d9ee746593cef034a957` |

The lists use repository-relative `.t` paths and are sorted. They can be
recreated by running the strict gate against both the clean control and the
archived dirty state, extracting witness failures, and comparing the sets with
`comm`.

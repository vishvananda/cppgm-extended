# Performance Tracker

Target benchmark:

```sh
./dev/cppgm++ -I dev/src -c -o /tmp/<label>-semantic_overload.o \
  benchmarks/self_compile/stable/semantic_overload.cpp
```

Primary goal: make the current worktree significantly faster than
`origin/main` for both wall time and instructions retired, without regressing
strict correctness.

## Baselines

| Label | Worktree/commit | Real | User | Sys | Max RSS | Peak Footprint | Instructions | Cycles | Notes |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| origin-main handoff | `/Users/vishvananda/cppgm-origin-main` `7cf50418` | 128.40s | 123.72s | 2.90s | 1,335,955,456 | 1,029,521,408 | 550,016,085,196 | 389,708,466,081 | Handoff baseline |
| origin-main local | `/Users/vishvananda/cppgm-origin-main` `7cf50418` | 108.94s | 105.89s | 2.38s | 1,306,320,896 | 1,031,475,200 | 550,376,290,090 | 345,175,154,216 | 2026-05-06 local `/usr/bin/time -lp`; object valid Mach-O x86_64 |
| safe local | `/Users/vishvananda/cppgm-safe` `75af1c08` | 117.39s | 114.11s | 2.52s | 1,398,042,624 | 1,118,666,752 | 591,313,609,731 | 373,836,440,294 | 2026-05-06 local `/usr/bin/time -lp`; object valid Mach-O x86_64 |
| current local baseline | `3e42ea35` | 188.37s | 184.66s | 2.59s | 1,245,372,416 | 979,611,648 | 1,202,519,687,220 | 626,888,392,935 | 2026-05-06 local `/usr/bin/time -lp`; object valid Mach-O x86_64 |
| current accepted recalibration | `f57a1631` | 104.45s | 101.13s | 2.36s | 1,292,390,400 | 1,017,823,232 | 503,344,246,556 | 330,573,050,073 | 2026-05-07 local `/usr/bin/time -lp` after rejected cache experiments; object valid Mach-O x86_64 |
| post-rebuild clean rerun | `008ed6b8` | 84.16s | 80.18s | 2.40s | 1,288,339,456 | 1,009,332,224 | 393,428,884,315 | 271,467,113,708 | 2026-05-07 clean-source rerun after full semantic rebuild; object byte-identical to step 34 output |
| post-rebase clean baseline | `579eb27f` | 82.37s | 79.19s | 2.42s | 1,210,736,640 | 939,655,168 | 343,994,658,538 | 258,716,428,731 | 2026-05-09 stats-enabled `/usr/bin/time -lp`; `make test-strict` with LowIR text compare passed |

## Change Log

| Step | Commit | Hypothesis / Change | Real | Instructions | Strict validation | Notes |
| --- | --- | --- | ---: | ---: | --- | --- |
| 0 | `3e42ea35` | Starting point after fast-forward; high repeated semantic/output work suspected | 188.37s | 1,202,519,687,220 | Not rerun yet | Valid object generated |
| 1 | `d79f9722` | Limit resolved-alias source-location search to the declaration token range | 97.67s | 494,773,254,914 | Passed `make test-strict` | Valid object; removes TU-wide token scan from late synthesized declaration parsing |
| 2 | `dc344e82` | Store diagnostic context thunks in typed lazy guards instead of `std::function` frames | 91.79s | 491,848,168,269 | Passed `make test-strict` | Valid object; `make -C dev cppgm++` also passed after include cleanup |
| 3 | `b5dd5b44` | Skip witness-only source-use location walks when no witness capture is active | 90.85s | 483,657,160,227 | Passed `make test-strict` | Valid object; byte-identical to step 2 no-witness output |
| 4 | `2b38d36a` | Skip type-lookup source-use location searches when source locations are inactive | 90.46s | 481,328,598,404 | Passed `make test-strict` | Valid object; byte-identical to the step 3 no-witness output |
| 5 | `bec49bf5` | Replace type-erased recursive template-argument combination callback with a local runner | 90.19s | 481,141,939,912 | Passed `make test-strict` | Valid object; byte-identical to the step 3 no-witness output |
| 6 | `a39d6313` | Defer function-definition closure witness metadata unless a witness session is active | 89.30s | 474,262,980,162 | Passed `make test-strict` | Valid object; byte-identical to the step 5 no-witness output |
| 7 | `6e7b4504` | Boost frontier replay baseline before repair work | 110.91s | 541,345,471,733 | Prior branch state | 2026-05-06 local run after replay; major regression lands around dependent non-type value-type/syntax preservation |
| 8 | `e277da1b` | Fix equivalent variable-template witness relocation to prefer the later public source use | n/a | n/a | Passed `make test-strict` | Correctness repair for pa22 strict witness after the Boost replay; no performance benchmark recorded for this witness-only change |
| 9 | `b7a41c0c` | Remove replay-added template argument/type text fallbacks that had structured replacements | 109.42s | 538,982,167,872 | Passed `make test-strict` | Valid Mach-O object; removes broad dependent type text scans, generated template-id text fallback, direct concrete member template-id text resolver, and synthesized leaf template-id fallback |
| 10 | `191840a1` | Reuse class-template reference fast path while the specialization epoch is unchanged | 111.02s | 536,380,803,172 | Passed `make test-strict` | Valid Mach-O object; preserves first partial-specialization/SFINAE selection and avoids reselecting unchanged hits |
| 11 | `7b3ada60` | Reuse completed forward class-template reference entries when the specialization epoch is unchanged | 110.77s | 534,921,888,998 | Passed `make test-strict` | Valid Mach-O object; preserves the initial forward/definition collection boundary and only reuses forward outputs after declaration collection is complete |
| 12 | `df161d75` | Allow the raw class-template reference cache to reuse completed forward entries | 109.89s | 533,453,678,487 | Passed `make test-strict` | Valid Mach-O object; exact raw argument/scope cache now shares the same declaration-complete and epoch guards as the resolved fast path |
| 13 | `1d92be55` | Precompute and compare template-argument fast-cache hashes before full key comparisons | 110.31s | 532,965,153,785 | Passed `make test-strict` | Valid Mach-O object; first run was 109.33s / 533,041,471,052 instructions, confirm run had noisier wall time but slightly lower instructions |
| 14 | `b187712b` | Cache ordinary function-template deduction results for stable argument type shapes | 110.18s | 530,085,126,257 | Passed `make test-strict` | Valid Mach-O object; cache is disabled for trace/witness capture and only stores cases whose actual argument types are complete or definitely non-class |
| 15 | `629e30fa` | Share namespace-scope function-template deduction cache entries across caller scopes when argument types are stable and non-local | 93.77s | 505,292,289,902 | Passed `make test-strict` | Valid Mach-O object; byte-identical to step 14 output; avoids re-deducing repeated libc++ operator candidates in distinct caller scopes |
| 16 | `ec520ae4` | Skip function-name scope splitting when the lookup name contains no operator spelling | 99.29s | 502,728,617,517 | Passed `make test-strict` | Valid Mach-O object; byte-identical to step 15 output; trims repeated non-operator function lookup normalization work in late overload paths |
| 17 | `a41a476a` | Cache the global scope-binding epoch so repeated scope fingerprints can return without walking parent scopes | 100.48s | 503,009,146,852 | Passed `make test-strict` | Valid Mach-O object; compared to the 2026-05-07 recalibration, max RSS dropped to 1,288,478,720 and peak footprint dropped to 1,012,273,152 |
| 18 | `4b7fe542` | Fast-path common type-transform aliases in direct type lookup | 101.39s | 502,293,767,148 | Passed `make test-strict` | Valid Mach-O object; byte-identical to step 17 output; slight wall tradeoff but instructions and max RSS improve, with max RSS 1,285,677,056 and peak footprint 1,012,359,168 |
| 19 | `ab9baae5` | Skip bound-type text lookup for simple identifier type names after exact local/bound checks | 101.62s | 501,466,206,267 | Passed `make test-strict` | Valid Mach-O object; byte-identical to step 18 output; max RSS 1,280,454,656 and peak footprint 1,012,686,848 |
| 20 | `4cf05d1f` | Evaluate libc++ `__libcpp_is_trivially_relocatable<T>::value` directly when marker lookup is determinate | 101.23s | 498,839,514,066 | Passed `make test-strict` | Valid Mach-O object; byte-identical to step 19 output; max RSS 1,278,631,936 and peak footprint 1,007,837,184 |
| 21 | `3f9d3925` | Cache pure Itanium mangling text canonicalization with bounded direct-mapped string memo tables | 96.57s | 455,464,901,562 | Passed `make test-strict` | Valid Mach-O object; byte-identical to step 20 output; max RSS 1,275,113,472 and peak footprint 1,011,171,328 |
| 22 | `810f65cb` | Resolve libc++ `__remove_const_ref_t<T>` aliases directly as remove-reference then remove-const | 95.59s | 455,945,779,584 | Passed `make test-strict` | Valid Mach-O object; byte-identical to step 21 output; max RSS 1,255,878,656 and peak footprint 1,011,478,528. Accepted as a wall/RSS win despite a small instruction increase |
| 23 | `5c792a9a` | Evaluate libc++ `__is_pair_v`, `__is_tuple_v`, and concrete `tuple_size` checks from structured class-template metadata | 95.64s | 452,897,302,423 | Passed `make test-strict` | Valid Mach-O object; byte-identical to step 22 output; max RSS 1,266,204,672 and peak footprint 1,009,905,664. Accepted for a 3.0B instruction reduction and lower peak footprint despite flat wall time and a 10.3MB max-RSS increase |
| 24 | `f83adab0` | Evaluate `is_error_code_enum<T>`/`is_error_condition_enum<T>` as false for arguments that cannot name user-defined specializations | 95.02s | 451,921,820,313 | Passed `make test-strict` | Valid Mach-O object; byte-identical to step 23 output; max RSS 1,274,736,640 and peak footprint 1,009,745,920. Wall and instructions improved; max RSS rose another 8.5MB while peak footprint stayed slightly lower |
| 25 | `d9ce43c9` | Retain function-template deduction cache argument types instead of caching naked `Type*` identities | 93.70s | 453,362,245,967 | Passed `make test-strict` | Correctness fix for intermittent `std::__to_address` misdeduction under phase stats; valid Mach-O object; byte-identical to step 24 output; max RSS 1,288,249,344 and peak footprint 1,011,355,648. Phase-stats repro also passed at 96.87s / 453,454,770,806 instructions |
| 26 | `e3e0c01c` | Add fine-grained semantic counters for deduction-cache keys, ADL collection, candidate identity strings, and partial ordering | 94.20s | 452,909,894,682 | Passed `make test-strict` | Instrumentation step; valid Mach-O object; max RSS 1,287,417,856 and peak footprint 1,012,133,888. Stats-enabled run was 95.72s / 453,521,487,298 instructions; plain and stats-enabled objects matched each other, but not step 25 because `semantic_metrics.h` layout changed |
| 27 | `badb9893` | Cache stable ADL associated namespace scopes and associated friend lookup results | 93.86s | 452,195,565,763 | Passed `make test-strict` | Valid Mach-O object; byte-identical to step 27 stats output; max RSS 1,271,758,848 and peak footprint 1,012,633,600. Stats-enabled run was 93.79s / 452,985,955,084 instructions with max RSS 1,265,070,080 |
| 28 | `888411b2` | Defer eager class reference-member collection until lookup actually needs it | 94.03s | 451,511,680,107 | Passed `make test-strict` | Valid Mach-O object; max RSS 1,267,912,704 and peak footprint 1,011,122,176. Avoids scanning most class bodies once for reference members and then again during full layout completion; wall was flat/noisy while instructions and memory improved |
| 29 | `8f89debb` | Cache repeated inline-namespace function-template entity comparisons | 88.04s | 413,748,648,934 | Passed `make test-strict` | Valid Mach-O object; byte-identical to step 28 output; max RSS 1,281,798,144 and peak footprint 1,011,683,328. Direct-mapped pair cache avoids repeatedly comparing function-template type patterns during overload-set deduplication |
| 30 | `e88cb4b4` | Skip function closure-event string work when no witness session is active | 85.82s | 396,817,402,600 | Passed `make test-strict` | Valid Mach-O object; byte-identical to step 29 output; max RSS 1,275,658,240 and peak footprint 1,009,561,600. Avoids constructing function lifecycle witness entity/decl strings that the witness logger would immediately discard |
| 31 | `f0ddf97a` | Use the function internal-symbol index when releasing duplicate symbol reservations | 80.81s | 394,897,272,302 | Passed `make test-strict` | Valid Mach-O object; byte-identical to step 30 output; max RSS 1,282,453,504 and peak footprint 1,009,885,184. Replaces an all-live-functions scan in `release_function_symbol_reservation` with the existing `functions_by_internal_symbol` bucket |
| 32 | `c0eb1413` | Gate qualified template lookup source-location searches when no source capture is active | 80.33s | 393,579,174,696 | Passed `make test-strict` | Valid Mach-O object; byte-identical to step 31 output; max RSS 1,285,709,824 and peak footprint 1,009,651,712. Mirrors the direct type-lookup source-use gate for structured qualified template type lookups |
| 33 | `7be88519` | Intersect lazy-body name-lookup snapshots by walking the smaller name set | 78.83s | 392,803,686,666 | Passed `make test-strict` | Valid Mach-O object; byte-identical to step 32 output; max RSS 1,278,590,976 and peak footprint 1,009,602,560. Reduces repeated pointer-hash probes while filtering saved parser lookup scopes for skipped header bodies |
| 34 | `008ed6b8` | Track emitted class member functions with pointer hash membership | 75.87s | 391,717,705,386 | Passed `make test-strict` | Valid Mach-O object; byte-identical to step 33 output; max RSS 1,285,193,728 and peak footprint 1,009,049,600. Replaces a per-class `std::set<FunctionBinding *>` used only for membership with an `unordered_set`, while keeping source-order output driven by the AST |
| 35 | `02408502` | Skip local named-type overlay scans when template arguments mention no candidate names | 79.19s | 392,644,440,322 | Passed `make test-strict` | Valid Mach-O object; byte-identical to step 34 output; max RSS 1,263,525,888 and peak footprint 1,009,934,336. Recalibrated clean runs after a full rebuild measured 84.16s / 393.43B and 86.76s / 395.82B, so this accepts the scoped early return as a win against the rebuilt baseline |
| 36 | `a66cdf2c` | Remove the nested dependent member type rescue after direct concrete member lookup | 88.38s | 343,281,998,565 | Passed `make test-strict` with LowIR compare | Stats-enabled run against the 2026-05-09 post-rebase baseline; max RSS 1,220,333,568 and peak footprint 940,183,552. Instructions and cycles improved while wall was noisy and RSS rose by about 9.6MB, so this is kept as a strict-clean fallback removal rather than a wall-time win |
| 37 | `b7769dcf` | Resolve dependent qualified-member owners only from exact bound type names | 106.60s | 342,758,460,955 | Passed `make test-strict` with LowIR compare | Replaces the broad current-specialization/recursive dependent-owner resolver with direct lookup in the template-bound type map. The two failures from fully disabling owner resolution were `T::type` and `Container::storage_type`, both already bound in instantiation scopes. Max RSS 1,190,977,536 and peak footprint 941,375,488; wall is noisy but instructions and RSS improve |
| 38 | `ab8aa125` | Skip the qualifier `<...>` text lookup probe before semantic-only qualifier fallback | 80.68s | 336,628,871,795 | Passed `make test-strict` with LowIR compare | The full semantic-only qualifier fallback removal was rejected at 349.66B instructions and 1,227,251,712 max RSS. Keeping that fallback but dropping the earlier `resolve_type_lookup_text` probe lowered resolve-template-argument calls to 297,597 and key builds to 42,420. Max RSS 1,218,379,776 and peak footprint 936,091,648 |
| 39 | `a42964fc` | Remove final alias-template substituted-text parse fallback | 73.22s | 336,527,503,690 | Passed `make test-strict` with LowIR compare | Deletes the final `parse_type_text_scoped` recovery after direct syntax, AST, and structural alias resolution have all failed. Strict stayed green, max RSS was 1,200,570,368 and peak footprint 937,267,200. The earlier qualified function-template broad fallback removal failed strict, and the narrower qualifier-template compact-text recovery removal was rejected at 336,744,539,778 instructions |
| 40 | `56a2c5c1` | Remove final template-deduction text-shape type fallback | 75.18s | 336,139,947,868 | Passed `make test-strict` with LowIR compare | Keeps bound-type and template-id shape deduction, but drops the final pattern/actual text type resolution branch. Max RSS was 1,215,578,112 and peak footprint 936,464,384; instructions and peak memory improved while wall stayed below 80s |
| 41 | `1f90c2ee` | Skip final emitted-definition validation acquisition in non-witness output | 144.50s | 333,928,290,315 | Passed `make test-strict` with LowIR compare | Wall/cycles were polluted by load, but object output stayed byte-identical and instructions improved. `semantic.validate_required_definitions` dropped to 13-15ms, validation class-info demand dropped from ~1.66M to 75, and instantiated class output-readiness calls dropped from 27,506 to 2,680. Max RSS 1,218,322,432 and peak footprint 937,963,520 on the confirm run; an ungated version failed 98 strict witness tests, showing validation still carries witness-observable acquisition side effects |
| 42 | `ba7df750` | Skip duplicate callee scans when closure reaches already-emitted function definitions | 69.94s | 334,003,233,846 | Passed `make test-strict` with LowIR compare | Function bodies already collect required callees at emission time; namespace-level closure was rediscovering those same function bodies. Object output stayed byte-identical, `fixpoint-callee-closure` class-info demand dropped from 73,826 to 0 and complete-class demand from 33,302 to 8, and required-definition requests dropped from 41,815 to 22,336. Max RSS 1,207,681,024 and peak footprint 936,464,384 on the confirm run |
| 43 | `d1541c72` | Add demand-bucketed output-seed attribution counters | 84.01s | 334,359,836,542 | Passed `make test-strict` with LowIR compare | Instrumentation step under semantic stats. New counters show `semantic.output_seed=24.81s`, 167 seed function bodies, 1,174 source statements, 31,288 expressions, 87,438 seed `complete_class_type` calls, 131 seed materializations, 14,296 reference-scope prepares, and 676 reference-member collections. Max RSS 1,224,265,728 and peak footprint 936,722,432 |
| 44 | pending | Defer output-seed member-object layout completion until class layout | 68.60s | 333,367,283,752 | Passed `make test-strict` with LowIR compare | Strict failures from the broad skip were all missing concrete class-valued field layouts. This keeps output-seed declaration/reference collection from recursively completing those field types, accepts deferrable concrete class members, and completes only still-incomplete fields at the class-layout boundary. Plain no-stats run was 72.88s / 333,779,217,916 instructions. Stats run: `semantic.output_seed=19.40s`, output-seed member-object complete-type calls `114 -> 0`, field-member-object complete-type calls `241 -> 191`, max RSS 1,209,888,768, peak footprint 938,598,400 |

## Investigation Notes

- Current branch still reproduces the handoff regression on 2026-05-06.
- Local same-machine comparison: current is 1.73x wall time and 2.18x
  instructions versus `origin-main`.
- `/Users/vishvananda/cppgm-safe` remains near `origin-main` at 1.08x wall time
  and 1.07x instructions. The regression was introduced after `75af1c08`.
- Memory remains below the origin-main handoff baseline, so the first target is
  repeated CPU work rather than allocation growth.
- A 2026-05-06 scratch run at `a30a6e86` is already slow but correct:
  176.49s and 1,165,099,520,421 instructions. That places the correctness
  enabling cliff before the late mangling/friend fixes.
- Sampling the regressed build showed late synthesized function output spending
  most sampled time in `source_location_for_last_qualified_member_start_before`
  through local variable declaration parsing. Range-limiting that search removed
  the sampled hotspot and cut the primary benchmark to 97.67s / 494.77B
  instructions.
- Several declaration/expression/overload paths still computed source-use
  anchors even when no witness session was active. Guarding those witness-only
  walks preserved no-witness object bytes and cut the benchmark to 90.85s /
  483.66B instructions.
- Type lookup still searched physical source locations for template-use anchors
  in no-witness/no-trace mode. Gating that lookup path preserved no-witness
  output and reduced instructions to 481.33B.
- Template call candidate expansion used a recursive `std::function` callback
  while exploring overload argument combinations. Replacing it with a local
  runner preserved output bytes and shaved a small amount of late semantic
  work.
- Function definition closure state still populated witness declaration
  locations and witness entity names in ordinary no-witness compilation. The
  semantic closure fields remain populated, but the witness-only fields now
  wait for an active witness session.
- After the Boost frontier replay, the stable compile benchmark regressed back
  to roughly 541-543B instructions. A strict-clean fallback cleanup removes the
  type/template argument text fallbacks that had structured replacements and
  lowers the stable compile to 538.98B instructions. A broader lazy-body
  namespace snapshot fixed one remaining qualified function-template fallback
  but cost ~550.1B instructions, so that path was discarded for now.
- The rebased branch baseline also had a pa22 strict witness regression where
  the first equivalent `has_max_size_v` source use at line 25 displaced the
  more specific failing-candidate use at line 30. Equivalent variable-use
  replacement now keeps the later public source location.
- A direct dependent `tuple_size<_Tp>` shortcut cut the stable compile to
  roughly 414B instructions but failed correctness by admitting an invalid
  `std::__1::get` instantiation. The safe version keeps the first
  partial-specialization/SFINAE selection and only reuses it while the class
  template specialization epoch is unchanged.
- Cached forward class-template references are reusable once top-level
  declaration collection is complete and the specialization epoch still
  matches. This avoids repeating a small amount of `tuple_size<_Tp>` forward
  output work without skipping the first selection that protects SFINAE
  correctness.
- Extending the raw class-template reference cache to those completed forward
  entries avoids the resolved-argument/key path for exact repeated raw uses.
  This remains much smaller than the unsafe direct shortcut, so the remaining
  wall-time gap is likely in repeated type/member checks and overload/template
  argument resolution outside the class-template reference cache.
- The template-argument fast cache is still net positive: disabling it measured
  110.27s / 536.53B instructions. A set-associative rewrite lost hit rate and
  regressed to 545.01B instructions, but precomputing the probe hash before the
  fast-cache scan lets nonmatching entries fail before full key comparison and
  saves roughly another 0.5B instructions.
- Repeated libc++ operator overload sets now dominate the hotspot summary after
  the class-template cache fixes. A conservative no-witness function-template
  deduction cache avoids redoing identical failed/successful deductions once the
  actual argument type graph is stable, saving another ~2.9B instructions on
  the stable self compile.
- The first deduction cache still keyed on caller scope identity, which blocked
  reuse for repeated namespace-scope libc++ operators in different function
  bodies. Omitting the caller scope only for namespace templates with stable
  non-local argument types preserves output bytes and drops the benchmark to
  505.29B instructions.
- Late-run sampling after the shared deduction cache shows repeated function
  lookup deduplication in inline-namespace overload sets. Avoiding a full
  `top_level_scope_split` for names that do not mention `operator` preserves
  output bytes and drops the benchmark to 502.73B instructions.
- A fresh accepted-baseline rerun on 2026-05-07 measured 104.45s / 503.34B
  instructions with 1,292,390,400 max RSS and 1,017,823,232 peak footprint.
  Candidate decisions after that point track max RSS and peak footprint, since
  lower instruction counts can still lose wall time through allocation pressure.
- The scope fingerprint global epoch avoids repeated parent-scope walks in
  `template_scope::scope_binding_fingerprint`. The accepted run improved wall
  time and memory against the fresh baseline while keeping instructions flat.
- Direct type lookup now resolves common libc++ transform aliases such as
  `remove_cv_t`, `remove_cvref_t`, `decay_t`, and `type_identity_t` without
  instantiating the alias template path. This preserved output bytes and reduced
  instructions/RSS, with a roughly one-second wall-time tradeoff on confirm.
- Simple identifier type-name lookup no longer consults bound-type text after
  the exact local-type and bound-type maps have already failed. That avoids a
  small repeated text path, preserves output bytes, and improves instructions
  and max RSS; peak footprint was effectively flat.
- After rebasing onto local `main` at `579eb27f`, the strict LowIR-compare gate
  passed and the stable compile counter baseline measured 82.37s /
  343.99B instructions with max RSS 1.21GB and peak footprint 939.7MB. Current
  high-volume counters remain `class-info-for-type-calls=6,778,698`,
  `scope-cache-key-calls=504,270`, `resolve-template-argument-calls=309,533`,
  and `output_seed=21,578ms`.
- Disabling all dependent-qualified owner resolution only failed two strict
  tests: a default non-type parameter type using `T::type`, and
  `bit_iterator<Container, false, 0>` using `Container::storage_type`. Tracing
  showed the earlier semantic data already existed in the instantiation scopes
  (`T=dependent_nontype_typifier<int>` and `Container=struct Container`). The
  kept fix uses that exact bound-type map directly and removes the broader
  current-specialization owner repair.
- Removing the whole qualifier semantic text fallback was strict-clean but
  regressed the stable compile to 349.66B instructions and 1.227GB max RSS by
  increasing class-info and complete-class work. The accepted narrower cut only
  removes the preliminary `<...>` `resolve_type_lookup_text` probe; the
  semantic fallback remains and the counters improve materially.
- libc++'s `__libcpp_is_trivially_relocatable<T>::value` is now evaluated from
  the structured type argument and the `__trivially_relocatable` member marker
  when that lookup is determinate. This avoids repeatedly instantiating the
  `is_same<T, typename T::__trivially_relocatable>` specialization path while
  preserving byte-identical output.
- A fresh accepted step-20 rerun measured 102.03s / 498.81B instructions with
  1,266,024,448 max RSS and 1,007,681,536 peak footprint. Candidate evaluation
  now records max RSS and peak footprint explicitly; a few MB of memory growth is
  acceptable only when wall time and instructions improve materially.
- Symbol-linkage text canonicalization was doing repeated template-component
  parsing and small string allocation while mangling repeated libc++ entities.
  Bounded direct-mapped memo tables for `canonical_component_text` and
  `named_substitution_key` cut the stable compile to 96.57s / 455.46B
  instructions. Max RSS remained below the step-20 accepted run, while peak
  footprint increased by about 3.3MB.
- libc++'s `__remove_const_ref_t<T>` aliases now use the same structured direct
  type-transform path as the existing remove-cvref/decay shortcuts. This
  preserved object bytes and reduced wall time/max RSS, but the instruction
  count moved up by about 0.5B, so future acceptance continues to look at wall,
  instructions, max RSS, and peak footprint together.
- libc++ internal variable templates `__is_pair_v` and `__is_tuple_v`, plus
  concrete `tuple_size<std::tuple<...>>::value`/`tuple_size<std::pair<...>>::value`
  comparisons, now resolve from existing class-template metadata. The shortcut
  is disabled during witness capture and does not perform text lookup fallback;
  it cuts instructions but did not move wall time materially on the first run.
- Standard `is_error_code_enum<T>` and `is_error_condition_enum<T>` checks now
  return false directly when `T` contains no named user-defined entity. This
  preserves user-specialization behavior for class and enum arguments while
  avoiding repeated libc++ `error_code` SFINAE work for built-in-shaped
  arguments.
- The intermittent `std::__to_address` failure under
  `CPPGM_SEMANTIC_PHASE_STATS=1` was consistent with function-template
  deduction cache entries matching a newly allocated type at an old `Type*`
  address. Retaining `TypePtr` values in the deduction cache key and the
  cacheability memo fixed the repro and preserved object bytes. This is a
  correctness fix first; the plain benchmark was faster on wall time but spent
  about 1.44B more instructions than step 24.
- Fine-grained stats now show deduction-cache volume directly:
  `function-template-deduction-cache-key-builds=96600`, `key-args=183508`,
  `hits=53093`, `misses=43507`, and `use-scope-sensitive-keys=8433`.
  Cacheability checks are mostly memo hits: `187695` checks, `179730` hits,
  `7965` full scans, and `3782` retained cache entries.
- ADL and overload-selection counters are now visible in the same stats line:
  `adl-associated-collections=7727`, `adl-associated-type-visits=14097`,
  `adl-associated-scope-outputs=13922`, `candidate-identity-builds=2173`,
  `candidate-identity-chars=961335`, `candidate-partial-order-comparisons=2680`,
  and `candidate-partial-order-acceptance-checks=2462`. These point to ADL
  scope collection as the larger remaining candidate-assembly volume.
- Stable ADL caching now reuses associated namespace scopes for repeated
  operand types and shares associated friend function/template lookup for each
  `(type, operator-name)` pair. The stats run reports
  `adl-associated-scope-cache-hits=11391`, `misses=2706`, `entries=2363`, and
  `uncacheable=343`. The combined ADL cache dropped stats-mode
  `complete-class-type-calls` from `109441` to `94424` and reduced the plain
  benchmark by about 0.71B instructions while lowering max RSS by 15.7MB.
- A 20-second sample taken during output seed showed `analyze_member_expression`
  forcing full class completion for field offsets. The hot stack included
  `populate_class_info`, `collect_class_simple_declaration`, and
  `maybe_complete_class_member_object_type`, often after the same class had
  already been scanned for reference members. Deferring namespace-scope
  reference-member scans preserves strict tests and saves about 0.68B
  instructions on the stable compile.
- The same sample also showed repeated inline-namespace function-template
  entity equivalence checks in overload-set deduplication. Caching stable
  `FunctionTemplateDecl*` pairs preserves output bytes and cuts the benchmark by
  roughly 37.8B instructions, moving the current accepted baseline to 88.04s.
- After the comparison cache, a fresh sample showed `binding_log_entity`,
  `class_witness_output_qualified_name`, and related witness lifecycle text
  helpers under ordinary `-c` output. Gating function closure-event logging when
  there is no witness session preserves output bytes and cuts another 16.9B
  instructions, with lower max RSS and peak footprint.
- A later sample also put `release_function_symbol_reservation` at the top of
  stack while duplicate function bindings were being merged/discarded. The
  registry already maintains `functions_by_internal_symbol`, so release now
  checks only that bucket instead of scanning every live function. This preserved
  output bytes, passed strict tests, and brought the stable compile to 80.81s.
- Structured qualified template type lookup still populated exact source-use
  anchors in ordinary no-witness/no-trace compiles. Gating the qualifier and
  leaf source-location walks the same way as direct type lookup preserved output
  bytes and shaved another 1.32B instructions, with a small max-RSS increase.
- A source sample after step 32 was allocation/string-lookup heavy and showed
  `snapshot_name_lookup_state` filtering saved parser scopes by probing the
  used-name set for every scope entry. Walking the smaller of the two sets
  instead preserves the same filtered snapshots, passes strict tests, and gets
  the stable compile back under the 80s target at 78.83s / 392.80B instructions.
- A nearby attempt to guard `maybe_complete_class_member_object_type` recursion
  by `ClassInfo*` instead of named-key strings preserved object bytes but
  regressed instructions to 395.44B and stayed above 80s, so it was reverted.
- Stats after step 33 showed `semantic.output_seed` and instantiated/late
  synthesized output as the remaining long poles. A candidate changed the local
  class-output `emitted` membership set from `std::set` to `std::unordered_set`
  and initially appeared to cut the stable compile to 75.87s / 391.72B
  instructions. Clean detached-worktree reruns later invalidated that result:
  the candidate measured 85.10s / 395.25B against 84.81s / 394.53B for its
  parent while preserving byte-identical output. The candidate was reverted; a
  related pointer-key unordered map for member node lookup was also rejected
  because it improved memory but regressed instructions to 394.50B.
- After abandoning broader source-location gating, several sampled micro-edits
  were rejected: direct lambdas in reference-member collection, pointer-based
  recursion guards for member-object completion, bulk class-function discard,
  typed `template_arguments_are_dependent` dispatch, shared LowIR parameter
  index construction, and unordered mangling substitutions. The common failure
  mode was lower or similar local-looking work but worse global instructions or
  code layout.
- A full semantic rebuild changed the practical clean-source timing band even
  with byte-identical output: clean reruns of step 34 measured 86.76s / 395.82B
  and 84.16s / 393.43B instructions. Future accept/reject decisions after this
  point compare against that rebuilt-source band unless a fresh clean rerun
  proves the old 75.87s result is reproducible.
- The local named-type overlay used during template instantiation already
  collects referenced identifiers from template argument text, but still walked
  use-scope local named types when that reference set was empty and no
  `__local_` marker existed. Returning before the scope walk preserves output
  bytes and is kept as a small clean-build instruction improvement. After the
  hash-set revert, the clean current-with-overlay/no-hash check measured
  84.96s / 393.31B instructions with byte-identical output; the earlier
  under-80s confirmation came from the same invalid incremental-build window as
  the hash-set result.

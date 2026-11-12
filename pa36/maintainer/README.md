# PA36 release measurements

`make -C pa36 ref-test` runs the course and student builds, enforces release
margins and generates `pa36/calibration.tsv` and `calibration-course.tsv`.
These are dated measurements tied to their recorded source, machine and
compiler configuration; the acceptance limits are defined in `gates.txt`.

A speed-only measurement does not regenerate the full release calibration.
Replace both snapshots only after a complete course and student release check
passes, preserving the generated values and provenance exactly. Current speed
measurements use matching optimization levels, including O3 on both sides.

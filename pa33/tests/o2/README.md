This bucket contains whole-function machine-backend `-O2` tests.

It intentionally repeats the local `-O1` surface under `-O2` and adds O2-only
layout/frame cases. Each `.t` file is LowIR input for `lowir2native -O2`, with
committed oracle sidecars for implementation status, dumped machine IR,
generated-program status, and stdout where relevant.

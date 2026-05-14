This bucket contains local machine-backend `-O1` tests.

Each `.t` file is LowIR input for `lowir2native -O1`. The committed oracle
sidecars check implementation exit status, dumped machine IR, generated-program
exit status, and generated-program stdout where relevant.

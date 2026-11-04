# Intel macOS

The optional macOS target uses x86-64 Mach-O objects and executables. It does
not support Apple Silicon. Linux x86-64 remains the required course target.

On an Intel Mac, select your installed Homebrew LLVM formula and keep that
toolchain throughout the build:

```sh
bash scripts/install_macos_llvm.sh 21
host_cxx="$(brew --prefix cppgm/ci/llvm@21)/bin/clang++"
make CXX="$host_cxx" CPPGM_HOST_CXX="$host_cxx"
dev/cppgm++ -O3 -c hello.cpp -o hello.o
"$host_cxx" hello.o -o hello
```

The compiler defaults to its host platform. `--target macos` and
`--target x86_64-apple-darwin` select Mach-O explicitly; `--target linux` selects
ELF. Automatic header discovery describes the build host. Cross-emission uses
explicit include directories and does not discover a foreign SDK. A native
Mac build is needed for normal macOS SDK and libc++ use.

As on Linux, `cppgm++`'s own link mode supplies the standalone course runtime.
Use the host Clang linker for programs needing the system C/C++ libraries.
Keep Clang in `CPPGM_HOST_CXX` during self-hosting:

```sh
make -C pa34 cppgm++-self CXX=../dev/cppgm++ CPPGM_HOST_CXX="$host_cxx"
make -C pa34 compare-cppgm++-inception CXX=../dev/cppgm++ CPPGM_HOST_CXX="$host_cxx"
```

## Validation

Run the same report as on Linux:

```sh
make test-report CXX="$host_cxx" CPPGM_HOST_CXX="$host_cxx"
```

The report runs portable assignment tests and the native Mach-O checks below.
Linux-specific object inspections and Cachegrind gates are listed as skips;
compilation and execution still run when only an inspection is unsupported.
Skips are reported separately from passed tests.

For focused Mach-O validation:

```sh
make -C pa24 test-macho CXX="$host_cxx" CPPGM_HOST_CXX="$host_cxx"
make -C pa25 test-macho CXX="$host_cxx" CPPGM_HOST_CXX="$host_cxx"
make -C pa26 test-macho CXX="$host_cxx" CPPGM_HOST_CXX="$host_cxx"
make -C pa27 test-macho CXX="$host_cxx" CPPGM_HOST_CXX="$host_cxx"
make -C pa28 test-macho CXX="$host_cxx" CPPGM_HOST_CXX="$host_cxx"
```

These checks are optional on Linux, where they check emission
and, when installed, use LLVM to inspect, link and verify debug information.
Intel Macs additionally execute the programs and test interoperability with
host-built objects, including exception unwinding and thread-local storage.
The **Intel macOS port** workflow runs on GitHub's `macos-26-intel` runner and
requires the full report and compares self-built and inception compilers with Homebrew LLVM 18 and 21,
matching the Ubuntu compiler majors. Both use a pinned Homebrew revision
with Intel bottles. Each job records its compiler's path and version. The
workflow runs on `main` and `v8macho` pushes and can be dispatched manually.

The Mach-O implementation adapts v3's format support to the current typed
backend. It emits objects directly and preserves the existing Linux path.

# index-cpp compatibility check

[index-cpp](https://github.com/kostya/index-cpp) is an upstream C++17 compiler
stress test with 50 checksummed workloads. Keep its source unchanged. The
reference compiler passes all 50 checks at `-O0`, `-O2` and `-O3` with GCC 15's
hosted headers and libraries, including the full `-O3` workload run.

From the repository root:

```sh
git clone https://github.com/kostya/index-cpp /tmp/cppgm-index-cpp
git -C /tmp/cppgm-index-cpp checkout 79e0b423094d420da37fb714f20c53fc35eeb2a3
make
dev/cppgm++ -std=c++17 -O3 -c /tmp/cppgm-index-cpp/index.cpp -o /tmp/cppgm-index.o
g++ /tmp/cppgm-index.o -lpthread -o /tmp/cppgm-index
TEST=1 /tmp/cppgm-index
```

Omit `TEST=1` for the full workload. The host compiler links the generated
object against its standard library; use the same host toolchain throughout.
This exercises selected C++14/17 features and does not establish full C++17
conformance. The check is optional and requires network access for the clone.

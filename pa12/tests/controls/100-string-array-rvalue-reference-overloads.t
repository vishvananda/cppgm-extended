// CWG 1467: the exact-bound rvalue-reference candidate cannot bind the
// literal lvalue; the padded array temporary selects the other. expect exit 0
// CXX11-021. Neighbor of LLVM21.1.8 clang/test/CXX/drs/cwg14xx.cpp,
// cwg1467::StringLiterals (Apache-2.0 WITH LLVM-exception; see upstream source).
int select(const char (&&)[4]) { return 1; }
int select(const char (&&)[5]) { return 0; }
int main() { return select({"abc"}); }

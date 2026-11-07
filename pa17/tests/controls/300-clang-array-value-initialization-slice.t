// Clang GH41441 C++11 array-new prefix; N3485 5.3.4/8.5. expect exit 0
// RUN: %clang --target=x86_64-pc-linux -S -fno-discard-value-names -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 %s -fsyntax-only -verify

namespace std {
  using size_t = decltype(sizeof(int));
};
void* operator new[](std::size_t, void* p) noexcept { return p; }
int initialization_failures;

// CHECK: call void @llvm.memset.p0.i64(ptr align 1 %x, i8 0, i64 8, i1 false)
// CHECK: call void @llvm.memset.p0.i64(ptr align 16 %x, i8 0, i64 32, i1 false)
template <typename TYPE>
void f()
{
    typedef TYPE TArray[8];

    TArray x;
    new(&x) TArray();
    for (unsigned i = 0; i != 8; ++i)
        if (x[i] != 0) ++initialization_failures;
}

template <typename T>
void f1() {
  int (*x)[1] = new int[1][1];
}
template void f1<char>();
void f2() {
  int (*x)[1] = new int[1][1];
}

int main()
{
    f<char>();
    f<int>();
    return initialization_failures;
}


// N3485 8.5.1p7: omitted array elements are value-initialized; expect exit 0.
// CXX11-007: reduced from LLVM llvmorg-21.1.8
// clang/test/CodeGenCXX/cxx11-initializer-aggregate.cpp (see NOTICE).
// The .limits sidecar bounds compilation independently of host load.
unsigned char values[1073741824ULL] = {0};

int main()
{
    if (values[0] != 0 || values[1] != 0 || values[1073741823ULL] != 0)
        return 1;
    values[1073741823ULL] = 7;
    return values[1073741823ULL] != 7;
}

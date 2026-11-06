// N3485 3.6.2p2, 7.1.5, 8.5.1p7: evaluate the explicit constexpr
// pointer initializer and retain the omitted zero range. expect exit 0
// CXX11-012: reduced from LLVM llvmorg-21.1.8
// clang/test/CodeGenCXX/cxx11-initializer-aggregate.cpp.
constexpr int* null_pointer() { return nullptr; }
int* values[134217728ULL] = {null_pointer()};
int target = 7;

int main()
{
    if (values[0] != nullptr || values[1] != nullptr || values[134217727ULL] != nullptr)
        return 1;
    values[134217727ULL] = &target;
    return *values[134217727ULL] != 7;
}

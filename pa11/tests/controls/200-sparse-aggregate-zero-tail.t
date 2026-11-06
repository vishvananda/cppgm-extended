// N3485 8.5.1p7,11: omitted subaggregates are value-initialized; expect exit 0.
// CXX11-007 neighbor of LLVM llvmorg-21.1.8
// clang/test/CodeGenCXX/cxx11-initializer-aggregate.cpp (see NOTICE).
// Nested arrays and brace elision must retain a compact implicit tail.
struct Row { unsigned char bytes[32768]; };
Row grid[32768] = {0};

int main()
{
    if (grid[0].bytes[0] || grid[0].bytes[32767] ||
        grid[1].bytes[0] || grid[32767].bytes[32767])
        return 1;
    grid[32767].bytes[32767] = 9;
    return grid[32767].bytes[32767] != 9;
}

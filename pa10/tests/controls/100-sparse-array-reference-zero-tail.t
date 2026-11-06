// N3485 8.5.4p3, 8.5.1p7: a list initializes a lifetime-extended array;
// its omitted tail remains compact during lowering. expect exit 0
// CXX11-011: LLVM cxx11-initializer-aggregate.cpp reduction (see NOTICE).
// Readonly native data must emit its bytes, so the bounded control uses 1 MiB.
const unsigned char (&values)[1048576] = {0};

int main()
{
    return values[0] != 0 || values[1] != 0 || values[1048575] != 0;
}

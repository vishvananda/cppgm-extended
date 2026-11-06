// N3485 8.5.4p3, 8.5.3p5, 12.2p5: a mutable lifetime-extended
// rvalue-reference array keeps its omitted tail compact. expect exit 0
// CXX11-011: LLVM cxx11-initializer-aggregate.cpp reduction (see NOTICE).
unsigned char (&&values)[1073741824ULL] = {0};

int main()
{
    if (values[0] != 0 || values[1] != 0 || values[1073741823ULL] != 0)
        return 1;
    values[1073741823ULL] = 7;
    return values[1073741823ULL] != 7;
}

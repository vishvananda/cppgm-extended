constexpr int value(int x) { return x * 3 + 1; }
int run() {
    int a[2][2] = {{value(1),value(2)},{value(3),value(4)}};
    int b[2][2] = {{value(1),value(2)},{value(3),value(4)}};
    constexpr int z[2][2] = {{value(-1)+2,0},{0,0}};
    a[0][0] = 99;
    return b[0][0] != 4 || a[1][1] != 13 || a == b || z[0][0] != 0 || z[1][1] != 0;
}
int main() { return run(); }

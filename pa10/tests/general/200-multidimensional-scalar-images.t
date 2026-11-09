// Automatic arrays retain identity, mutability, effects and exact representations.
int effects;
int value() { return ++effects; }
void escape(int*);
void image() { int values[2][3] = {{1,2,3},{4,5,6}}; escape(&values[0][0]); }
void zero() { int values[2][3] = {}; escape(&values[0][0]); }
void one_zero() { int values[6] = {}; escape(values); }
int run() {
    int a[2][3] = {{1,2,3},{4,5,6}};
    int b[2][3] = {{1,2,3},{4,5,6}};
    a[0][0] = 99;
    int bad = b[0][0] != 1 || a == b;
    int z[2][3] = {};
    for (int i = 0; i != 2; ++i)
        for (int j = 0; j != 3; ++j) bad += z[i][j] != 0;
    int dynamic[2][2] = {{value(),value()},{value(),value()}};
    bad += effects != 4 || dynamic[0][0] != 1 || dynamic[1][1] != 4;
    volatile int v[2][2] = {{1,2},{3,4}};
    bad += v[0][1] + v[1][0] != 5;
    double signs[2][2] = {{-0.0,0.0},{0.0,-0.0}};
    bad += !(1.0 / signs[0][0] < 0.0) || !(1.0 / signs[1][1] < 0.0);
    int* pointers[2][2] = {{&a[0][0],nullptr},{nullptr,&b[1][2]}};
    bad += *pointers[0][0] != 99 || pointers[0][1] != nullptr || *pointers[1][1] != 6;
    return bad != 0;
}
void escape(int* p) { *p += 1; }
int main() { return run(); }

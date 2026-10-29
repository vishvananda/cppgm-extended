typedef int Values[3];
typedef Values Matrix[2];
int main() {
    int* p = new Values();
    if (p[0] || p[1] || p[2]) return 1;
    delete[] p;
    const int* q = new const Values{4,5};
    if (q[0] != 4 || q[1] != 5 || q[2]) return 2;
    delete[] q;
    Values* matrix = new (Matrix){{1,2,3},{4,5,6}};
    if (matrix[0][2] != 3 || matrix[1][2] != 6) return 3;
    delete[] matrix;
    Values* explicit_count = new Values[2]();
    explicit_count[1][2] = 42;
    if (explicit_count[0][2] || explicit_count[1][2] != 42) return 4;
    delete[] explicit_count;
    return 0;
}

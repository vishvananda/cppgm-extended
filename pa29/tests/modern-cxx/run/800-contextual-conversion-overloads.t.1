struct Count {
    operator unsigned() const { return 5; }
    operator unsigned() const volatile { return 7; }
};
struct Pointer {
    int *p;
    operator int *() const { return p; }
    operator int *() const volatile { return p; }
};
int main() {
    Count n;
    int *p = new int[n];
    for (unsigned i = 0; i != 5; ++i) p[i] = i;
    switch (n) { case 5: break; default: return 1; }
    Pointer q = {p};
    delete [] q;
    return 0;
}

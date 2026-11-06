// N3485 8.5.4p3, 12.2p5, 3.6.3p1: a loop-constructed static array
// reference keeps its elements alive and finalizes all of them. expect exit 0
int made, destroyed, bad;
struct Verify {
    ~Verify() { if (made != 33 || destroyed != 33 || bad) __builtin_abort(); }
} verify;
struct Element {
    int id, value;
    Element(int v = 0) : id(++made), value(v) {}
    ~Element() { if (id != made - destroyed) bad = 1; ++destroyed; }
};
const Element (&values)[33] = {{7}};
int main()
{
    return made != 33 || destroyed != 0 || values[0].value != 7 ||
           values[1].value != 0 || values[32].id != 33;
}

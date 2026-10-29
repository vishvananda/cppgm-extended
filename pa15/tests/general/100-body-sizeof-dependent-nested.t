template<unsigned long N> struct Size { enum { value = N }; };
struct Item { long value; };
template<class T> int widths(T p) {
    return Size<sizeof(p->value) + sizeof(p[0].value) + sizeof(*p)>::value;
}
int effects;
long observed;
long& observe() { ++effects; return observed; }
template<class T> int independent(T) {
    return Size<sizeof(observe())>::value;
}
int main() {
    Item item = {0};
    return widths(&item) + independent(0) - 32 + effects;
}

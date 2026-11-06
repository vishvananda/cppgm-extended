// N3485 8.5.1p7,11: value-initialization still checks constructor access;
// expect reject. CXX11-007 neighbor: a tail cannot evade its type's checks.
struct Element {
    Element(int) {}
private:
    Element() {}
};
Element elements[4] = {1};

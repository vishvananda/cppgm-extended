// N3485 5.19p2: a constexpr reference does not make nonliteral temporary
// elements constant-initializable. expect reject
int calls;
struct Element {
    int value;
    Element() : value(++calls) {}
};
constexpr const Element (&values)[3] = {};
int main() { return values[0].value; }

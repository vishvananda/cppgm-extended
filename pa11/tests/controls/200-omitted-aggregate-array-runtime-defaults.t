// N3485 8.5.1, 12.6.2: omitted aggregate arrays execute runtime member constructors. expect exit 0
int calls;
struct Leaf { int value; Leaf() : value(7) { ++calls; } };
struct Pair { Leaf left, right; };
struct Outer { Pair values[2]; };
Outer value = {};
int main() { return calls != 4 || value.values[0].left.value != 7 || value.values[1].right.value != 7; }

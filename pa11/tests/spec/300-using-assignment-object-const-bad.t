// N3485 13.3.1p4-p5: the implicit object conversion must retain const.
struct Base { int operator=(int) { return 1; } };
struct Value : Base { using Base::operator=; };
int main() { const Value value = {}; value = 1; }

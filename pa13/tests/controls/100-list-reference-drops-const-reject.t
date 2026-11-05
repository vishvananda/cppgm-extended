// N3485 8.5.4p3, 8.5.3p5: direct list-reference binding cannot drop const.
// expect reject
struct Abstract { virtual int value() const = 0; };
struct Concrete : Abstract { int value() const { return 7; } };
void take(Abstract&) {}
int main() { const Concrete object; take({object}); }

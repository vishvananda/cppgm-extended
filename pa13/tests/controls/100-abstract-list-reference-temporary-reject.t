// N3485 10.4p2, 8.5.4p3: a reference declaration's empty list also needs
// an abstract temporary, which cannot be created. expect reject
struct Abstract { virtual int value() const = 0; };
int main() { const Abstract& value = {}; }

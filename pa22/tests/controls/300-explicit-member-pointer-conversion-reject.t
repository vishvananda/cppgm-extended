// expect reject
struct Root { int value; };
struct Explicit { explicit operator Root*() const; };
int read(Explicit value) { return value->*&Root::value; }

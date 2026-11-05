// N3485 10.4p2, 8.5.4p3: a call's braced reference argument cannot create
// an abstract temporary. expect reject
struct Abstract { virtual int value() const = 0; };
void take(const Abstract&) {}
int main() { take({}); }

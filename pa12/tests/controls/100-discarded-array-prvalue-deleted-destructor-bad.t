// expect reject
// I13: N3485 12.2/12.4: a discarded array prvalue still requires each element destructor.
struct A { ~A() = delete; }; int main() { typedef const A Array[2]; Array{}; }

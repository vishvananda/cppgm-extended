struct A { int value; int run() { return [*this] { return ++value; }(); } };
int main() {}

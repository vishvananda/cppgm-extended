struct A { void run() { auto closure=[this,*this] {}; } };
int main() {}

struct A { A()=default; A(const A&)=delete; void run() { auto closure=[*this] {}; } };
int main() {}

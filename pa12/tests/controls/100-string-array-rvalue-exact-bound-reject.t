// 8.5.4p3 and 8.5.3: an exactly related literal array binds directly,
// so an rvalue reference cannot bind this literal lvalue. expect reject
const char (&&text)[4] = {"abc"};
int main() { return 0; }

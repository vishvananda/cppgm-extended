// N3485 14.3p1: an ordinary concrete argument list exceeds a fixed head; expect reject
template<class A, class B> struct fixed {};
fixed<int, char, long> invalid;
int main() {}

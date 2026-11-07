// N3485 focus: 14 [temp], 9.2/9.4; PA17 documented variable-template extension.
// expect reject
template<class T> struct S { template<class U> U member; };
S<int> instance;
int main() { return 0; }

// N3485 focus: 14 [temp], 9.2/9.4; PA17 documented variable-template extension.
// expect reject
struct S { template<class T> int member; };
int main() { return 0; }

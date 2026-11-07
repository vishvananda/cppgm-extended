// N3485 focus: 14 [temp], 9.2/9.4; PA17 documented variable-template extension.
// expect exit 0
struct S { template<class T> int member(T value) { return sizeof(T) + value; } };
int main() { S s; return s.member(3) == sizeof(int) + 3 && s.member(char(2)) == 3 ? 0 : 1; }

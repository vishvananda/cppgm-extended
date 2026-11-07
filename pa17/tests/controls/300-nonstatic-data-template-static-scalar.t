// N3485 focus: 14 [temp], 9.2/9.4; PA17 documented variable-template extension.
// expect exit 0
struct S { template<class T> static int member; };
template<class T> int S::member = sizeof(T);
int main() { S::member<int> += 3; return S::member<int> == sizeof(int) + 3 && S::member<char> == 1 ? 0 : 1; }

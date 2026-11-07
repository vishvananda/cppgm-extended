// N3485 focus: 14 [temp], 9.2/9.4; PA17 documented variable-template extension.
// expect exit 0
template<class T> int member = sizeof(T);
int main() { member<int> += 3; return member<int> == sizeof(int) + 3 && member<char> == 1 ? 0 : 1; }

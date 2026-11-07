// N3485 focus: 14 [temp], 9.2/9.4; PA17 documented variable-template extension.
// expect exit 0
template<class T> struct S { template<class U> static int member; };
template<class T> template<class U> int S<T>::member = sizeof(T) * 10 + sizeof(U);
int main() { return S<char>::member<int> == 10 + sizeof(int) && S<int>::member<char> == sizeof(int) * 10 + 1 ? 0 : 1; }

// N3485 14.5.7/14.5.3; one written expansion is not a known scalar argument; expect exit 0
template<class A> using Identity = A;
template<class... T> int inspect(Identity<T...> value) { return value; }
int main() { return inspect<int>(7) - 7; }

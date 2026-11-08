// expect reject
template<class T> struct Friend {
 template<int N> friend void action(T value) noexcept(sizeof(decltype(value)) < 1);
};
template<int N> void action(int) noexcept {}
Friend<int> object;
int main() { return 0; }

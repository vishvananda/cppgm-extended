// expect exit 0
template<class T> struct Friend {
 template<int N> friend void action(T) noexcept(N > 0);
};
template<int N> void action(int) noexcept(N > 0) {}
Friend<int> object;
int main() { action<0>(1); action<1>(1); return 0; }

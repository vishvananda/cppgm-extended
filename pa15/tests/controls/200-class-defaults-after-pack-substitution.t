// expect exit 0
// N3485 14.1p9, 14.5.3: once expansion fixes the positions, append the
// class defaults before interning a specialization or binding its members.
template<class T = int, class U = T, int N = sizeof(U)> struct Box {
    U value;
    char bound[N];
    Box() : value(7), bound() {}
};
template<class... T> Box<T...> make(T...) { return Box<T...>(); }
template<int N = 3, int M = N + 1> struct Numbers { char bound[M]; };
template<int... N> Numbers<N...> numbers() { return Numbers<N...>(); }
int main() {
    return make(1).value != 7 || make().value != 7 || make(1L).value != 7 ||
        sizeof(make(1).bound) != sizeof(int) ||
        sizeof(make(1L).bound) != sizeof(long) ||
        sizeof(numbers<2>().bound) != 3 || sizeof(numbers<>().bound) != 4;
}

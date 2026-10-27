// expect exit 0
// CWG 1591, 14.8.2.1 [temp.deduct.call]: nonempty lists deduce an array
// element type and a named bound. Fixed bounds are not deduced from the list.
template<class T, int N> int count(const T (&)[N]) { return N; }
template<class T, int N> int choose(const T (&)[N]) { return N; }
template<class T, int N> int choose(T (&&)[N]) { return N + 10; }
template<class T> int fixed(const T (&a)[4]) { return a[0] + a[3]; }
template<class T, int R, int C> int matrix(const T (&a)[R][C]) {
    return 100 * R + 10 * C + a[1][1];
}
int main() {
    return count({3, 5, 7}) != 3 || choose({3, 5, 7}) != 13 ||
           fixed({9}) != 9 || matrix({{1, 2}, {3, 4}}) != 224;
}

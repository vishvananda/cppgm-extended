template<class T> int size() { if constexpr(sizeof(T)==sizeof(int)) return 7; else return T::missing; }
int main() { return size<int>()==7?0:1; }

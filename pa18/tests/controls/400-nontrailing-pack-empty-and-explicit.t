// expect exit 0
template<class... T> int take(T..., int middle, T...) { return middle; }
int main() { return take(7) == 7 && take<int>(1, 7, 2) == 7 && take<int,int>(1, 2, 7, 3, 4) == 7 ? 0 : 1; }

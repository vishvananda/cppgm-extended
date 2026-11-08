// expect error
template<class... T> void take(T..., int, T...) {}
int main() { take(0, 0, 0); }

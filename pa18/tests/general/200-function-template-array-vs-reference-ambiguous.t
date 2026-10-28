template<class T> int select(T[]) { return 1; }
template<class T, int N> int select(T (&)[N]) { return 2; }
int main(){ char a[3]={}; return select(a); }

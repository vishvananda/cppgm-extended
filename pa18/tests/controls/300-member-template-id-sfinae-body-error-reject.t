// expect reject
struct absent {};
template<class T> int call(T) { int(*p)(int)=&T::template fun<int>;return 0; }
int main(){return call(absent());}

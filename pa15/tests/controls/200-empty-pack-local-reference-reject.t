// A reference cannot be value-initialized by an empty expansion; expect reject
template<class T,class... A>void bad(A... a){T& value(a...);(void)value;}
int main(){bad<int>();}

// A scalar cannot be initialized from two expanded values; expect reject
template<class T,class... A>T bad(A... a){T value(a...);return value;}
int main(){return bad<int>(1,2);}

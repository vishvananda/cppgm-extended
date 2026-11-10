template<class T>int run(T value){int n=4;auto outer=[&]{auto inner=[&](auto v){return n+v+value;};return inner(2);};return outer();}
int main(){return run(1)==7?0:1;}

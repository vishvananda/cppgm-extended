struct Box {using value_type=int;};
Box make_box();
int target(int n){return n+1;}
template<class T> int call(int n){using Alias=decltype(make_box());return((int(*)(Alias::value_type))target)(n);}
int main(){return call<void>(41)!=42;}

// N3485 12.8p15-p16,5.19p2: an empty mutable member has no stored value to read.
// expect reject
struct A{mutable int n;};constexpr A a={3};constexpr A b=a;int main(){return 0;}

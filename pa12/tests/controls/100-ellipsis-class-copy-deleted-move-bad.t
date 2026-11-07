// N3485 5.2.2p7,4.1p2: evaluated class glvalues initialize an ellipsis argument temporary.
// expect reject
void sink(...);struct A{A()=default;A(const A&)=default;A(A&&)=delete;int n;};void f(){A a{};sink(static_cast<A&&>(a));}int main(){return 0;}

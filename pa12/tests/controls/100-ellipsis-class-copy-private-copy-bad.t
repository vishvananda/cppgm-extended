// N3485 5.2.2p7,4.1p2: evaluated class glvalues initialize an ellipsis argument temporary.
// expect reject
void sink(...);struct A{A()=default;int n;private:A(const A&)=default;};void f(){A a{};sink(a);}int main(){return 0;}

// N3485 5.2.2p7,4.1p2: evaluated class glvalues initialize an ellipsis argument temporary.
// expect exit 0
int sink(...){return 2;}struct A{A()=default;int n;friend int test();private:A(const A&)=default;};int test(){A a{};return sink(a);}int main(){return test()!=2;}

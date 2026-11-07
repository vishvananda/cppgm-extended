// N3485 5.2.2p7,4.1p2: evaluated class glvalues initialize an ellipsis argument temporary.
// expect exit 0
int sink(...){return 2;}struct A{A()=default;A(A&&)=default;int n;};int main(){A a{};return sink(static_cast<A&&>(a))!=2;}

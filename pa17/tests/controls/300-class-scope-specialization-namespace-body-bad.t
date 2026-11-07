// expect reject
// A namespace-written explicit class definition owns an ordinary body, even when nested in an instance.
template<class T> struct Outer { template<class U> struct Inner; };
template<> template<> struct Outer<int>::Inner<int> { int unused(){return missing_name;} };
int main(){}

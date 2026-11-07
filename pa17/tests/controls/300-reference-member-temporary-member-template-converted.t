// Adopted CWG1696: an instantiated member-template constructor cannot bind its reference member to a converted temporary.
// expect reject
struct Value { Value(const char*) {} };
template<class T> struct Holder { T reference; template<class U> Holder(U&& input) : reference(input) {} };
Holder<Value&&> holder("value");

// Adopted CWG1696: the same member-template constructor may bind an actual object.
// expect exit 0
struct Value { int number; Value(int n) : number(n) {} };
template<class T> struct Holder { T reference; template<class U> Holder(U&& input) : reference(input) {} };
int main() { Value value(3); Holder<Value&> holder(value); value.number=7; return &holder.reference==&value && holder.reference.number==7 ? 0 : 1; }

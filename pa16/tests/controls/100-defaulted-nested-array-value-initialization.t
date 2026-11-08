// expect exit 0
// CWG1360 resolved by P2448R2: defaulted constexpr suitability is distinct from actual initialized values.
struct Element { int n; explicit Element()=default; }; struct Object { Element elements[2]; explicit Object()=default; }; constexpr Object object=Object(); static_assert(object.elements[0].n==0 && object.elements[1].n==0,"nested zero initialization"); int main(){return object.elements[1].n;}

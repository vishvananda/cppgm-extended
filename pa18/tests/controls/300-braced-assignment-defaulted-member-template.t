// expect exit 0
// I13: N3485 14.8.2/13.3: nondeduced braced RHS uses the member-template default before explicit conversion candidates.
struct Value { int n; };
template<class T> struct Optional {
 int value;
 Optional() : value(0) {}
 template<class U> explicit Optional(U) : value(99) {}
 template<class U=T> void operator=(U item) { value=item.n; }
};
int main() { Optional<Value> object; object={7}; if(object.value!=7) return 1; object={Value{9}}; return object.value!=9; }

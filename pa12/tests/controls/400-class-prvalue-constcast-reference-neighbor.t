// expect exit 0
// CWG1965/2879 preserves const_cast to an rvalue reference from a materialized class prvalue.
struct Object { int n; }; int main() { Object&& object=const_cast<Object&&>(Object{7}); object.n=9; return object.n!=9; }

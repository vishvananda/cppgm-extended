// expect reject
// I13: N3485 12.8/11: mutable subobject copy overload resolution selects an inaccessible constructor template.
struct A { A() {} private: template<class T> A(T&); }; struct B { mutable A member; }; int main() { B first; B second(first); }

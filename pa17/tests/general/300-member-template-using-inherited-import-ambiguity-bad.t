// VALIDATION: compile-fail
// An imported member template is not owned by the class importing it.
struct A { template<class T> int erase(const T&) { return 1; } };
struct B : A { template<class T> long erase(const T&) { return 2; } };
struct Derived : B { using A::erase; using B::erase; };
int main() { Derived object; return object.erase(0); }

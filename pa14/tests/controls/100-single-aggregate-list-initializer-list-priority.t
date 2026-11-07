// CWG1467/2137; whole-object and member/list-constructor neighbors; expect exit 0
namespace std { template<class T> struct initializer_list { const T* p; unsigned long n; }; } int lists; struct E { E() {} E(std::initializer_list<E>) { ++lists; } }; int main() { E a; E b{a}; return lists != 1; }

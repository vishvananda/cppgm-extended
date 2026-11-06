// N3485 7.2p2, CWG1638: an ordinary opaque enum declaration cannot use a qualified name.
// expect reject
template<class T>struct A{enum class E;}; template<>enum class A<int>::E; enum class A<int>::E; int main(){}

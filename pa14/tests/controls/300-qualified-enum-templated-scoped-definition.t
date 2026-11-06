// N3485 14.5.1.4 and 7.2p4: an opaque member enum of a class template may be defined out of class.
// expect exit 0
template<class T>struct A{enum class E;}; template<class T>enum class A<T>::E{zero}; int main(){return int(A<int>::E::zero);}

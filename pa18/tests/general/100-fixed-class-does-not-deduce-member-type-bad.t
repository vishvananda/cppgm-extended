struct Fixed {};
template<class T>struct Dep { typedef T Type; };
template<class Signature>struct Ref { Ref(int) {} };
template<class T>int emit(Ref<void(Fixed&,const typename Dep<T>::Type&)>) { return 7; }
int main() { return emit(0); }

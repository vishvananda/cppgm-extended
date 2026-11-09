struct Fixed {};
template<class T>struct Dep { typedef T Type; };
template<class Signature>struct Ref { Ref(int) {} };
template<class T>int emit(T value,Ref<void(Fixed&,const typename Dep<T>::Type&)>) { return value; }
int main() { return emit(7,0)==7?0:1; }

template<class T> struct Base { static constexpr int value = 7; };
template<class T> struct Derived : Base<T> { int get() const; };
template<class T> int Derived<T>::get() const { return value; }
int main() { return Derived<int>{}.get(); }

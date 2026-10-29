// The current class and identical incomplete returns need no completion.
struct Base { virtual Base* self() = 0; };
template<class T> struct Self : Base {
    Self* self() { return this; }
};
template<class T> struct Forward;
struct Interface { virtual Forward<int>* get() = 0; };
struct Same : Interface { Forward<int>* get() { return 0; } };
template<class T> struct Dormant {
    typename T::absent value;
};
struct Ordinary { Dormant<int>* get(); };
int main() {
    Self<int> s;
    Base& b = s;
    Same same;
    Interface& i = same;
    return b.self() != &s || i.get() != 0 || sizeof(Ordinary) != 1;
}

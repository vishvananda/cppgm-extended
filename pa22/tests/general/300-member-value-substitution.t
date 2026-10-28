struct Object {
    int field;
    int function() const { return 7; }
};
struct Apply {
    template<class Base, class T, class Derived>
    auto operator()(T Base::* member, Derived& object) const
        -> decltype(object.*member) { return object.*member; }
    template<class Base, class T, class Derived, class... Args>
    auto operator()(T Base::* member, Derived& object, Args... args) const
        -> decltype((object.*member)(args...)) { return (object.*member)(args...); }
};
struct IndirectApply {
    template<class Base, class T, class Derived>
    auto operator()(T Base::* member, Derived* object) const
        -> decltype(object->*member) { return object->*member; }
    template<class Base, class T, class Derived, class... Args>
    auto operator()(T Base::* member, Derived* object, Args... args) const
        -> decltype((object->*member)(args...)) { return (object->*member)(args...); }
};
template<class T> T& get();
template<class T> auto query(int) -> decltype((get<T>().* &T::function), char{});
template<class> long query(...);
int main() {
    Object object{9};
    return Apply{}(&Object::function, object) != 7
        || Apply{}(&Object::field, object) != 9
        || IndirectApply{}(&Object::function, &object) != 7
        || IndirectApply{}(&Object::field, &object) != 9
        || sizeof(decltype(query<Object>(0))) != sizeof(long);
}

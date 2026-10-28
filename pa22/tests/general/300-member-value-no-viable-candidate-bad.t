struct Object { int function() const; };
template<class C, class T> auto field(T C::* member, C& object)
    -> decltype(object.*member) { return object.*member; }
int main() { Object object; return field(&Object::function, object); }

struct Friend;
struct Base {
    friend struct Friend;
    int value() const { return 7; }
};
struct Derived : Base {
protected:
    using Base::value;
};
struct Friend : Derived {
    // Base's friendship does not remove Derived's using-declaration access.
    static int get(Derived& object) { return object.value(); }
};

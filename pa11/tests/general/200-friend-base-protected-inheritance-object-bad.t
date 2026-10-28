struct Friend;
struct Base {
    friend struct Friend;
protected:
    int value() const { return 7; }
};
struct Derived : protected Base {};
struct Friend : Derived {
    // Friendship with Base does not bypass Derived's protected inheritance.
    static int get(Derived& object) { return object.value(); }
};

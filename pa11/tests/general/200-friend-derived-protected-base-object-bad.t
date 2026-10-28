struct Friend;
struct Base {
protected:
    int value() const { return 7; }
};
struct Derived : Base { friend struct Friend; };
struct Friend : Derived {
    // Friendship in Derived does not grant access through a Base object.
    static int get(Base& object) { return object.value(); }
};

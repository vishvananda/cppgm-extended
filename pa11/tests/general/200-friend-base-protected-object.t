struct Friend;
struct Base {
    friend struct Friend;
protected:
    int value() const { return 7; }
};
struct Derived : Base {};
struct Friend : protected Derived {
    static int get(Derived& object) { return object.value(); }
    struct Nested {
        static int get(Derived& object) { return object.value(); }
    };
};
int main() {
    Derived object;
    return Friend::get(object) != 7 || Friend::Nested::get(object) != 7;
}

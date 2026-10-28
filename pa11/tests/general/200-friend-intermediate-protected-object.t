struct Friend;
struct Base {
protected:
    int value() const { return 7; }
};
struct Middle : protected Base { friend struct Friend; };
struct Derived : Middle {};
struct Friend : Derived {
    static int get(Derived& object) { return object.value(); }
};
int main() { Derived object; return Friend::get(object) != 7; }

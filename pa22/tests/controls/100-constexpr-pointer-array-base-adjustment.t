// N3485 4.10p3, 5.19: a constexpr pointer prefix applies its
// nonzero derived-to-base adjustment exactly once. expect exit 0
struct Lead { int first; };
struct Base { int value; };
struct Derived : Lead, Base {
    constexpr Derived() : Lead{7}, Base{11} {}
};
Derived object;
constexpr Derived* address() { return &object; }
Base* values[4] = {address()};

int main()
{
    return values[0] != static_cast<Base*>(&object) || values[0]->value != 11 ||
        values[1] != nullptr || values[3] != nullptr;
}

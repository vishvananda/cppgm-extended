struct Base { enum Mode { Disabled, Enabled }; };
struct Derived : Base {};
int main() { Derived value; return value.Mode::Disabled; }

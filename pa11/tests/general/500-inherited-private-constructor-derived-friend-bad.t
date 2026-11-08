class Base { Base(int) {} };
struct Factory;
struct Derived:Base { using Base::Base; friend struct Factory; };
struct Factory { static Derived make() { return Derived(7); } };
int main() { Factory::make(); }

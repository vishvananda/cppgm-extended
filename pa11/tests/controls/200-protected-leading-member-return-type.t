// expect exit 0
class Base { protected: struct Node {}; };
class Derived : public Base { public: Node* make() const; };
Base::Node* Derived::make() const { return nullptr; }
int main() { Derived value; return value.make() != nullptr; }

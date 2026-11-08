struct Factory;
class Base {
  friend struct Factory;
  int value;
protected:
  Base(int n):value(n) {}
public:
  int get() const { return value; }
};
class Derived:public Base { using Base::Base; };
class Further:public Derived { using Derived::Derived; };
class PrivateBase {
  friend struct Factory;
  int value;
  PrivateBase(int n):value(n) {}
public:
  int get() const { return value; }
};
class PrivateDerived:public PrivateBase { using PrivateBase::PrivateBase; };
struct Factory {
  static int first() { Derived value(7); return value.get(); }
  static int second() { Further value(8); return value.get(); }
  static int third() { PrivateDerived value(9); return value.get(); }
};
int main() { return Factory::first()+Factory::second()+Factory::third()==24?0:1; }

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
  static Derived first() { return Derived(7); }
  static Further second() { return Further(8); }
  static PrivateDerived third() { return PrivateDerived(9); }
};
int main() { return Factory::first().get()+Factory::second().get()+Factory::third().get()==24?0:1; }

// VALIDATION: compile-pass
// Same-signature virtuals from unrelated base roots must keep independent
// final overriders. Derived2::f does not override Abstract::f.

struct Abstract
{
   virtual void f() = 0;
};

struct Derived1 : public Abstract
{
   virtual void f() {}
};

struct Abstract2
{
   virtual void f() = 0;
};

struct Derived2 : public Abstract2
{
   virtual void f() {}
};

struct Combined : public Derived1, public Derived2
{
};

int main()
{
   Combined combined;
   static_cast<Derived1*>(&combined)->f();
   static_cast<Derived2*>(&combined)->f();
   return 0;
}

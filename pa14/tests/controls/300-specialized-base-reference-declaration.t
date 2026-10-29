// N3485 10.2, 14.6.2: a fixed base specialization includes its dependent
// bases when classifying an inherited local reference declaration; expect exit 0
struct Context { typedef Context inner_context_type; typedef Context outermost_context_type; int value; };
template<class D, class C> struct Root {
  typedef typename C::inner_context_type context_type;
  typedef typename context_type::outermost_context_type outermost_context_type;
  outermost_context_type object;
  outermost_context_type& context() { return object; }
};
template<class D, class C> struct Middle : Root<D, C> {};
struct Derived : Middle<Derived, Context> {
  Derived() { outermost_context_type& object = context(); object.value = 7; }
};
int main() { Derived object; return object.context().value != 7; }

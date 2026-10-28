// A bare member template is not a substituted typename. It must reject an
// immediate-context overload, while actual nested types and template-template
// arguments retain their distinct meanings.
template<class T> struct wrap {};
template<template<class> class T> struct template_wrap {};

template<class T> char type_test(wrap<T>*, wrap<typename T::item>* = 0);
long type_test(...);
template<class T> char template_test(wrap<T>*, template_wrap<T::template item>* = 0);
long template_test(...);

template<class T> struct type_trait {
  static const bool value = sizeof(type_test((wrap<T>*)0)) == sizeof(char);
};
template<class T> struct template_trait {
  static const bool value = sizeof(template_test((wrap<T>*)0)) == sizeof(char);
};

struct ordinary { typedef int item; };
struct incomplete { struct item; };
struct class_template {
  template<class T = int> struct item { typename T::never_instantiate bad; };
};
struct alias_template { template<class T> using item = T; };
struct missing {};
struct variable { int item; };

static_assert(type_trait<ordinary>::value, "a typedef is a type");
static_assert(type_trait<incomplete>::value, "an incomplete nested class is a type");
static_assert(!type_trait<class_template>::value, "a class template is not a type");
static_assert(!type_trait<alias_template>::value, "an alias template is not a type");
static_assert(!type_trait<missing>::value, "a missing member is not a type");
static_assert(!type_trait<variable>::value, "a data member is not a type");
static_assert(template_trait<class_template>::value, "a member template argument");
static_assert(template_trait<alias_template>::value, "an alias template argument");
static_assert(!template_trait<ordinary>::value, "a typedef is not a template");

template<class T> typename T::item* choose(int) { return 0; }
template<class T> char choose(...) { return 7; }
int main() {
  return choose<class_template>(0) != 7 || choose<alias_template>(0) != 7 ||
    choose<ordinary>(0) != nullptr;
}

template<class A, class B> struct same { static constexpr bool value = false; };
template<class A> struct same<A, A> { static constexpr bool value = true; };

int value = 3;
const int fixed = 7;
decltype(auto) copy = value;
decltype(auto) reference = (value);
decltype(auto) constant = fixed;
decltype(auto) constant_reference = (fixed);

decltype(auto) result_value() { return value; }
decltype(auto) result_reference() { return (value); }
decltype(auto) result_void() { return; }

template<class T> decltype(auto) forward_value(T&& x) {
  return static_cast<T&&>(x);
}

struct Record { int& reference; const int member; };
Record record = { value, 9 };
decltype(auto) member_value() { return record.member; }
decltype(auto) member_reference() { return record.reference; }

static_assert(same<decltype(copy), int>::value, "copy strips expression category");
static_assert(same<decltype(reference), int&>::value, "parentheses preserve lvalue");
static_assert(same<decltype(constant), const int>::value, "named entity type");
static_assert(same<decltype(constant_reference), const int&>::value, "const lvalue");
static_assert(same<decltype(result_value()), int>::value, "return named entity");
static_assert(same<decltype(result_reference()), int&>::value, "return lvalue");
static_assert(same<decltype(result_void()), void>::value, "deduced void");
static_assert(same<decltype(forward_value(value)), int&>::value, "template lvalue");
static_assert(same<decltype(forward_value(4)), int&&>::value, "template xvalue");
static_assert(same<decltype(&member_value), const int(*)()>::value, "member declared type");
static_assert(same<decltype(member_reference()), int&>::value, "reference member");

int main() {
  auto access = []() -> decltype(auto) { return (value); };
  static_assert(same<decltype(access()), int&>::value, "lambda return placeholder");
  return 0;
}

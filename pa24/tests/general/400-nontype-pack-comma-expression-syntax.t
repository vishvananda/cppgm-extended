// N3485 focus: [temp.variadic] pack expansion in template arguments.
// Expanding a non-type pack expression must preserve the expression AST so the
// instantiated argument is evaluated structurally, not by reparsing text.
template<class A, class B>
struct is_same {
  static const bool value = false;
};

template<class A>
struct is_same<A, A> {
  static const bool value = true;
};

template<bool... B>
struct all_dummy {};

template<bool... B>
struct all : is_same<all_dummy<B...>, all_dummy<((void)B, true)...> > {};

int main()
{
  return all<true>::value && !all<false>::value ? 0 : 1;
}

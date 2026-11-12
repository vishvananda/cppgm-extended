#if !__has_builtin(__make_integer_seq) || !__has_builtin(__type_pack_element)
#error missing builtin template advertisement
#endif
template<class T, T... Values> struct Sequence {
 static constexpr unsigned size = sizeof...(Values);
};
typedef __make_integer_seq<Sequence, unsigned long, 7> Seven;
typedef __type_pack_element<2, short, float, unsigned long, void*> Selected;
static_assert(__is_same(Selected, unsigned long), "pack element type");
static_assert(sizeof(Selected) == 4, "target long layout");
static_assert(sizeof(__builtin_va_list) == 4, "target va_list layout");
extern "C" int observe() { return Seven::size + sizeof(Selected) + sizeof(__builtin_va_list) + 2; }

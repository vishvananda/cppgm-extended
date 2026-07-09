namespace boost_like {
namespace json {

struct key_value_pair
{
  long value;
};

struct object
{
  using index_t = int;

  struct table
  {
    static inline table * allocate();
  };
};

inline object::table * object::table::allocate()
{
  static_assert(alignof(key_value_pair) >= alignof(index_t),
                "enclosing alias should be visible in lazy nested member body");
  return 0;
}

} // namespace json
} // namespace boost_like

template<bool Reservable>
struct base {
  static const bool reservable = Reservable;
};

template<bool Reservable>
struct option {
  template<class Base>
  struct pack : Base {
    static const bool reservable = Reservable;
  };
};

typedef option<true>::pack<base<false> > packed;

static_assert(packed::reservable == true, "nested owner value");

int main()
{
  return packed::reservable ? 0 : 1;
}

template<class T>
T && move_like(T & t)
{
  return static_cast<T &&>(t);
}

struct policy {};

template<class>
struct ops;

template<>
struct ops<policy> {
  template<class Iter>
  static Iter && iter_move(Iter & i)
  {
    return move_like(i);
  }
};

template<class T>
void sift(T * p)
{
  T top(ops<policy>::iter_move(*p));
}

void f()
{
  struct local {
    local() {}
    local(const local &) {}
    local(local &&) {}
  };
  local a[1];
  sift(a);
}

int main()
{
  return 0;
}

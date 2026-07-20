template<int Dummy = 0>
struct declared {
  void* value;
  static const int cmp2 = 0;
  friend void operator>(int, const declared&) {}
};

struct undeclared {
  declared<> dummy[2];
};

template<int>
struct resolve;

template<>
struct resolve<sizeof(declared<>)> {
  static const int cmp1 = 0;
};

template<>
struct resolve<sizeof(undeclared)> {
  template<int>
  struct cmp1 {
    static const int cmp2 = 0;
  };
};

extern undeclared local_args;

int main()
{
  declared<resolve<sizeof(local_args)>::cmp1<0>::cmp2> local_args;
  declared<resolve<sizeof(local_args)>::cmp1<0>::cmp2> local_args;
  return 0;
}

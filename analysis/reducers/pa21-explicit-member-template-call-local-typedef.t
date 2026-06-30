struct worker
{
  template<class T>
  int run()
  {
    return sizeof(T) == sizeof(int) ? 3 : 4;
  }
};

template<class U>
int call_member_template()
{
  typedef U value_type;
  worker w;
  return w.template run<value_type>();
}

int main()
{
  return call_member_template<int>();
}

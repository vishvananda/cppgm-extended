// VALIDATION: compile-pass
// An instantiated class template reuses the PA17 virtual-destructor lifetime
// path.

template<class T>
struct owner
{
  virtual ~owner()
  {
  }
};

owner<int> value;

int main()
{
  return 0;
}

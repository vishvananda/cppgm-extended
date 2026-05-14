// VALIDATION: compile-fail
// N3485 focus: 14.2 [temp.names], 14.6.4 [temp.dep.res]
// Expected: dependent member template calls require the `template` keyword.

template<typename Tag>
struct Box
{
  template<typename T>
  T cast(T value)
  {
    return value;
  }
};

template<typename Tag>
int run(Box<Tag> & box)
{
  return box.cast<int>(4);
}

int main()
{
  Box<int> b;
  return run(b);
}

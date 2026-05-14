template<typename T>
struct Box
{
  template<typename U>
  static int cast(U value)
  {
    return static_cast<int>(value) + 1;
  }

  int run() const
  {
    return []() { return cast<int>(4); }();
  }
};

int main()
{
  Box<int> box;
  return box.run() == 5 ? 0 : 1;
}

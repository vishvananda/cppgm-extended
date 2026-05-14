struct Counter
{
  static thread_local int value;
};

thread_local int Counter::value = 4;

int main()
{
  return Counter::value == 4 ? 0 : 1;
}

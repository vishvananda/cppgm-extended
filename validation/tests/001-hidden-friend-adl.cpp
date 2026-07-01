// VALIDATION: compile-pass
// N3485 focus: 3.4.2 [basic.lookup.argdep], 11.3 [class.friend]

struct Box
{
  int value;

  friend int get(Box b)
  {
    return b.value;
  }
};

int main()
{
  Box b = {7};
  return get(b) == 7 ? 0 : 1;
}

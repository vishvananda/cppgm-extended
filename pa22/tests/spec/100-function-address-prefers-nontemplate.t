// N3485 focus: 13.4 [over.over], 14.8.2.2 [temp.deduct.funcaddr]
// When a target function type identifies both an ordinary function and a
// function-template specialization, overload resolution selects the ordinary
// function.

int choose(int)
{
  return 0;
}

template<class T>
int choose(T)
{
  return 1;
}

typedef int (*choice)(int);

choice selected = choose;

int main()
{
  return selected(1);
}

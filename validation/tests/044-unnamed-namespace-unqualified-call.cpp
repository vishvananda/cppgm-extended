namespace
{
int bump(int value)
{
  return value + 1;
}
}

int run()
{
  return bump(2);
}

int main()
{
  return run() == 3 ? 0 : 1;
}

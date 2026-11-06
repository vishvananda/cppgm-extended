int observed;

int first(int value, ...) { return value; }

int observe(int value)
{
  observed += value;
  return value;
}

int use_first(int value)
{
  int result = first(value, observe(3), 2.5, observe(5));
  return result + observed;
}

int main()
{
  return use_first(11) != 19 || first(23) != 23;
}

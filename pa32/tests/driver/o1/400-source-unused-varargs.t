int observed;

int first(int value, ...) { return value; }

int observe(int value)
{
  observed += value;
  return value;
}

int use_first(int value)
{
  return first(value, observe(3), 2.5, observe(5)) + observed;
}

int main()
{
  return use_first(11) != 19 || first(23) != 23;
}

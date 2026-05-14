struct S
{
  int x;
};

const S & first(const S * values)
{
  return values[0];
}

int main()
{
  S values[1] = {{5}};
  return first(values).x == 5 ? 0 : 1;
}

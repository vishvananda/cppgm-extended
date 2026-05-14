double nans_double()
{
  return __builtin_nans("");
}

float nans_float()
{
  return __builtin_nansf("");
}

long double nans_long_double()
{
  return __builtin_nansl("");
}

int main()
{
  return 0;
}

template <class T>
void print_expression(const char*, T)
{
}

int current_rounding_mode()
{
  return __builtin_flt_rounds();
}

int main()
{
  print_expression("FLT_ROUNDS", (__builtin_flt_rounds()));
  return current_rounding_mode();
}

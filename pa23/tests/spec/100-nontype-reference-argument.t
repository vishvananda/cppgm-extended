// VALIDATION: run-pass
// N3485 focus: 14.3.2 [temp.arg.nontype]

int global_value = 7;

template<int & Ref>
struct ref_counter
{
  static void bump()
  {
    ++Ref;
  }
};

int main()
{
  ref_counter<global_value>::bump();
  return global_value == 8 ? 0 : 1;
}

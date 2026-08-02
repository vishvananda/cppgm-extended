// VALIDATION: compile-pass
// A scalar reference used as a built-in pointer subscript is converted to its
// referent value before the selected class object's member is accessed.

struct element
{
  int value;
};

int read(element * elements, int & subscript)
{
  return elements[subscript].value;
}

int main()
{
  element values[1] = {{7}};
  int subscript = 0;
  return read(values, subscript) == 7 ? 0 : 1;
}

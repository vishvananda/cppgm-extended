// VALIDATION: run-pass
// N3485 focus: hosted integration sentinel

#include <map>

struct value_box
{
  int value;

  value_box() : value(0) {}
};

int main()
{
  std::map<unsigned char, value_box> values;
  unsigned char key = 1;
  values[key].value = 3;
  return values[key].value == 3 ? 0 : 1;
}

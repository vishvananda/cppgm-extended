int object;
constexpr int& reference = object;
int& second = reference;
static_assert(&reference == &object, "constant address of static object");

int main()
{
  second = 7;
  return &reference == &object && &second == &object && object == 7 ? 0 : 1;
}

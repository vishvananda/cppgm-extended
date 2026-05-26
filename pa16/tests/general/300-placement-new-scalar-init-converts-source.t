#include <new>

int main()
{
  float f = 0.0f;
  int i = 7;
  new (&f) float(i);
  return 0;
}

#include <cfloat>
#include <cmath>

#ifndef NAN
#error NAN should remain defined after cfloat and cmath
#endif

float nan_value()
{
  return NAN;
}

int main()
{
  return 0;
}

#include "400-nested-catch-effects.helper.h"
int unknown(int mode) {
  Guard active(5);
  switch (mode % 5) {
    case 0: return mode + 9;
    case 1: throw First{mode * 3};
    case 2: throw Second{mode * 7};
    case 3: throw mode * 11;
    default: return mode - 2;
  }
}

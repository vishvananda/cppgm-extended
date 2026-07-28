#pragma once

extern int initializations;

inline int & shared_value()
{
  static int value = ++initializations;
  return value;
}

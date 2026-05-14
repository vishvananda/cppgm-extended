#include "optimization_level.h"

#include <string>

using namespace std;

int normalize_optimization_level(int level)
{
  if(level <= 0) {
    return 0;
  }
  if(level == 1) {
    return 1;
  }
  return 2;
}

bool parse_optimization_level_arg(const string & arg, int & level)
{
  if(arg.size() < 2 || arg[0] != '-' || arg[1] != 'O') {
    return false;
  }
  if(arg == "-O0") {
    level = 0;
    return true;
  }
  if(arg == "-O" || arg == "-O1") {
    level = 1;
    return true;
  }
  level = 2;
  return true;
}

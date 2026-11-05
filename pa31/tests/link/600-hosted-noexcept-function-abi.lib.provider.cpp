#include "600-hosted-noexcept-function-abi.h"
int accept(Throwing) { return 3; }
int accept(Nonthrowing) { return 7; }
int check_rtti(const std::type_info& info) { return info != typeid(Nonthrowing); }
int catch_pointer(Nonthrowing pointer, void(*raise)(Nonthrowing)) {
  try { raise(pointer); }
  catch (Throwing recovered) { return recovered(); }
  catch (...) { return -1; }
  return -2;
}
void throw_pointer(Nonthrowing pointer) { throw pointer; }
template int kind<true>(Nonthrowing);
template int kind<false>(Throwing);

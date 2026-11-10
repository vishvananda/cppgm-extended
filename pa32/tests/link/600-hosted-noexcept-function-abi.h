#include <typeinfo>
using Throwing = int(*)();
using Nonthrowing = int(*)() noexcept;
int accept(Throwing);
int accept(Nonthrowing);
int check_rtti(const std::type_info&);
int catch_pointer(Nonthrowing, void(*)(Nonthrowing));
void throw_pointer(Nonthrowing);
template<bool B> int kind(int(*)() noexcept(B)) { return B ? 7 : 3; }
extern template int kind<true>(Nonthrowing);
extern template int kind<false>(Throwing);

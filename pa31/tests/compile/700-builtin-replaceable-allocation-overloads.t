#include <new>
#if __has_builtin(__builtin_operator_new) < 201802L || __has_builtin(__builtin_operator_delete) < 201802L
#error The allocation builtins must advertise the complete replaceable overload set
#endif
void exercise()
{
  void* a = __builtin_operator_new(32);
  __builtin_operator_delete(a);
  void* b = __builtin_operator_new(32, std::nothrow);
  __builtin_operator_delete(b, std::nothrow);
  void* c = __builtin_operator_new(64, std::align_val_t(64));
  __builtin_operator_delete(c, 64UL, std::align_val_t(64));
  void* d = __builtin_operator_new(64, std::align_val_t(64), std::nothrow);
  __builtin_operator_delete(d, std::align_val_t(64), std::nothrow);
  void* e = __builtin_operator_new(32);
  __builtin_operator_delete(e, 32UL);
}

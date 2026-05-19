#if defined(__APPLE__)
#include <TargetConditionals.h>

#if !defined(TARGET_OS_MAC)
#error missing TargetConditionals macros
#endif
#endif

int main()
{
  return 0;
}

#ifdef __linux__
#define SYS_futex SYS_futex_time64
#include <sys/syscall.h>
static_assert(SYS_futex == __NR_futex, "system header replaces the earlier fallback");
#endif
int main() { return 0; }

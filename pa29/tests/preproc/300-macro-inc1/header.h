#define NEXT <header.h>
#if __has_include_next(ID(NEXT))
next_present
#else
#error macro include-next header was not found
#endif
#include_next NEXT

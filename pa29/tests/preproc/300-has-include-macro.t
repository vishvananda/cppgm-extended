#define HEADER "300-present.h"
#if !__has_include(HEADER)
#error existing macro-named header was not found
#endif
macro_present
#define ALIAS HEADER
#define ID(x) x
#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)
#define EMPTY
#if __has_include(ID(ALIAS)) && __has_include(STRINGIZE(300-present.h)) && __has_include(EMPTY HEADER EMPTY)
nested_present
#else
#error nested macro header was not found
#endif
#define ABSENT "300-missing.h"
#if __has_include(ABSENT)
#error missing header was found
#else
macro_absent
#endif
#if ID(__has_include(ID(HEADER))) && ID(__has_include(ABSENT) == 0)
argument_present
#else
#error probe in an enclosing macro argument failed
#endif
#define PROBE(x) __has_include(x)
#if PROBE(HEADER)
replacement_present
#else
#error probe from a replacement list failed
#endif
#define ANGLE <header.h>
#if __has_include(ID(ANGLE))
angle_present
#else
#error macro angle header was not found
#endif
#define header missing
#if __has_include(<header.h>) && !__has_include(ANGLE)
direct_angle_preserved
#else
#error direct and computed angle headers were not distinguished
#endif
#undef header
#include ANGLE

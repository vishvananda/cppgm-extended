#ifndef HEADER_VALUE
#error command-line definitions must precede forced headers
#endif
#ifdef REMOVED_VALUE
#error command-line undefinitions must precede forced headers
#endif
#define FIRST_FORCED_HEADER 1
const int forced_value = HEADER_VALUE;

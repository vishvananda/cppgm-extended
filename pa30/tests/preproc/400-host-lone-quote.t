// A lone quote is a token of its own in hosted text a directive skips or
// never expands; only a token that reaches later phases is an error.
#define UNUSED don't
#if 0
This isn't compiled, and neither is a " alone.
#error can't happen
#endif
#ifdef NOT_DEFINED
#error it's "not defined
#endif
int after = 1;

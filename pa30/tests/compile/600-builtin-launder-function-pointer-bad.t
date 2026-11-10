void target();
void (*invalid)() = __builtin_launder(&target);

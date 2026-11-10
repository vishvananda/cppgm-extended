extern "C" long absolute_target[4] __attribute__((visibility("hidden")));
extern "C" const long* asm_absolute_symbol() {
    const long* result;
    asm("movabs %1, %0" : "=r"(result) : "i"(&absolute_target[0]));
    return result;
}
extern "C" const long* asm_absolute_symbol_addend() {
    const long* result;
    asm("movabs %1, %0" : "=r"(result) : "i"(&absolute_target[2]));
    return result;
}

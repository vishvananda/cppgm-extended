void rejected() { asm goto("jmp %l[missing]" : : : : missing); }

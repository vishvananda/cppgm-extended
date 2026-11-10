int rejected() { asm goto("jmp %l[after]" : : : : after); int value = 3; after: return value; }

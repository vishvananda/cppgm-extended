void rejected() { asm goto("jmp %0" : : : : done); done: ; }

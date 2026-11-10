// Module section directives preserve their ordered assembly state.
asm(".section .cppgm_note,\"\",@progbits\n.byte 3\n");
int main() { return 0; }

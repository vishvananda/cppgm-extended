// Section state may not leak from a basic asm definition into compiler code.
asm(".section .cppgm_note,\"\",@progbits\n.byte 3\n");
int main() { return 0; }

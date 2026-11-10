// Existing text sections retain their executable section identity.
asm(".pushsection .text,\"\",@progbits\n.byte 0\n.popsection\n");
int main() { return 0; }

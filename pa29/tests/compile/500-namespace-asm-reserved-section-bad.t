// Predefined executable sections cannot be reinterpreted as nonloaded data.
asm(".pushsection .text,\"\",@progbits\n.byte 0\n.popsection\n");
int main() { return 0; }

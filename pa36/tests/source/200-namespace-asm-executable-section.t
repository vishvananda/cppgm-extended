// Executable user sections may contain ordinary instructions.
asm(".pushsection .cppgm_code,\"ax\",@progbits\nnop\n.popsection\n");
int main() { return 0; }

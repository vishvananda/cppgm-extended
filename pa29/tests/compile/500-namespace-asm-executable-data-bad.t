// Namespace asm's supported data form cannot silently discard executable code.
asm(".pushsection .cppgm_code,\"ax\",@progbits\nnop\n.popsection\n");
int main() { return 0; }

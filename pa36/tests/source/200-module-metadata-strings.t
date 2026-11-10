// Namespace basic asm contributes nonloaded section data, with C++ literal
// concatenation and GAS string escapes decoded once before object emission.
asm(".pushsection \".debug_cppgm_data\", \"MS\", %progbits,1\n"
    ".ascii \"\\4cppgm.probe\\nhello\\n\"\n"
    ".byte 0\n.popsection\n");
namespace metadata {
asm(".pushsection \".cppgm_note\", \"\", @progbits\n"
    ".ascii \"A\\\"\\\\\\101\"\n.byte 0,255\n"
    ".pushsection .cppgm_nested,\"\",@progbits\n"
    ".asciz \"\\x41\"\n.string \"B\"\n.popsection\n"
    ".byte 2\n.popsection\n");
}
asm(".pushsection .cppgm_note,\"\",@progbits\n"
    ".section .cppgm_note,\"\",@progbits\n.byte 3\n.popsection\n");
asm("");
int main() { return 0; }

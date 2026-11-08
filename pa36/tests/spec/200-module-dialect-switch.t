declare function @module_dialect(%value : i64) -> i64
declare function @module_dialect_second(%value : i64) -> i64
assembly @mixed att none 1 0 0 2 {
 operand ptr 0 -1 1 0 0 { alternative 8 0 -1 {} }
 operand ptr 1 -1 1 0 0 { alternative 8 0 -1 {} }
 text ".text\n.globl "
 use 0 1 0 0
 text "\n"
 use 0 1 0 0
 text ":\nmov %rdi,%rax\njmp 2f\n1:\n.intel_syntax noprefix\nadd rax,3\nret\n.att_syntax prefix\n2:\nadd $5,%rax\njmp 1b\n.intel_syntax prefix\n.balign 16\n"
 text ".pushsection .data\n.att_syntax noprefix\n.popsection\n.globl "
 use 1 1 0 0
 text "\n"
 use 1 1 0 0
 text ":\nmov rdi,rax\n.intel_syntax prefix\nlea %rax,[%rdi+%rdi*2+5]\n.att_syntax noprefix\nlea 2(rax),rax\n.intel_syntax prefix\nadd %rax,1\n.att_syntax prefix\nret\n"
}
assembly_global @mixed(@module_dialect,@module_dialect_second)

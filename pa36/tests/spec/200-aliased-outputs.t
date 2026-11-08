assembly @outputs intel none 1 0 0 2 {
operand i64 -1 0 2 0 0 {
 alternative 1 0 -1 {
group (rax:0)
}
}
operand i64 -1 1 2 0 0 {
 alternative 1 0 -1 {
group (rcx:0)
}
}
text "mov rax, 0x1122334455667788; mov rcx, 0x99aabbccddeeff00"
}
function @alias(%out : ptr) -> void {
 block ^entry:
 asm @outputs(%out,%out)
 return void
}

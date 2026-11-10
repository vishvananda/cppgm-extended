assembly @query_asm att none 1 0 1 5 {
operand i32 -1 0 2 0 0 {
 alternative 1 0 -1 {
group (eax:0)
}
}
operand i32 -1 1 2 0 0 {
 alternative 1 0 -1 {
group (r10d:0)
}
}
operand i32 -1 2 2 0 0 {
 alternative 1 0 -1 {
group (ecx:0)
}
}
operand i32 -1 3 2 0 0 {
 alternative 1 0 -1 {
group (edx:0)
}
}
operand i32 4 -1 1 0 0 {
 alternative 0 0 0 {
}
}
text "xchgq %rbx, %r10; cpuid; xchgq %rbx, %r10"
}
function @query(%leaf : i32, %out : ptr) -> void {
 block ^entry:
 %p0 = index i32 %out, 0
 %p1 = index i32 %out, 1
 %p2 = index i32 %out, 2
 %p3 = index i32 %out, 3
 asm @query_asm(%p0,%p1,%p2,%p3,%leaf)
 return void
}

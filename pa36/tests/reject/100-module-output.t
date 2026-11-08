declare global @result : i64
assembly @invalid intel none 1 0 0 1 {
 operand i64 -1 0 2 0 0 { alternative 1 0 -1 { group (rax) } }
 text "nop"
}
assembly_global @invalid(@result)

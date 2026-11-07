function @product(%a : v128, %b : v128) -> v128 {
  block ^entry:
    %r = vector pclmulqdq %a, %b, 256
    return v128 %r
}

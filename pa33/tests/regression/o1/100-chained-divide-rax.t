# A quotient divided again stays in rax, and a word divide reads its parameters
# unextended. A loaded pointer that outlives its block shares the register of
# the address it is loaded through.
function @chain(%x : i32, %a : i32, %b : i32, %c : i32) -> i32 [binding=strong, no_inline=yes] {
  block ^entry:
    %p = binary div i32 %x, %a
    %q = binary div i32 %p, %b
    %r = binary div i32 %q, %c
    return i32 %r
}

function @seen(%p : ptr) -> i32 [binding=strong, no_inline=yes] {
  block ^entry:
    %v = load typed i32 %p
    return i32 %v
}

function @through(%s : ptr) -> i32 [binding=strong, no_inline=yes] {
  block ^entry:
    %q = load typed ptr %s
    branch %q, ^some, ^none

  block ^some:
    %r = call i32 @seen(%q)
    return i32 %r

  block ^none:
    return i32 0
}

global @value : i32 = 3
global @cell : ptr = addr @value

function @main() -> i32 [role=entry] {
  block ^entry:
    %q = call i32 @chain(120, 2, 3, 4)
    %t = call i32 @through(@cell)
    %s = binary sub i32 %q, 2
    %d = binary sub i32 %s, %t
    return i32 %d
}

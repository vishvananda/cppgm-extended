# Values kept across a call take RBX, R14 and R15 before R13 and R12: as the
# base of a memory operand R13 needs a displacement byte where there is no
# displacement, and R12 a SIB byte. `@three` reads through three pointers
# after its call, from RBX, R14 and R15. `@five` keeps five: the last two
# are in R13 and R12.
function @touch(%p : ptr) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %v = load i32 %p
    %w = binary add i32 %v, 1
    store i32 %w, %p
    return void
}

function @three(%a : ptr, %b : ptr, %c : ptr) -> i32 [binding=strong, no_inline=yes] {
  block ^entry:
    call void @touch(%a)
    %x = load i32 %a
    %y = load i32 %b
    %z = load i32 %c
    %s1 = binary add i32 %x, %y
    %s2 = binary add i32 %s1, %z
    return i32 %s2
}

function @five(%a : ptr, %b : ptr, %c : ptr, %d : ptr, %e : ptr) -> i32 [binding=strong, no_inline=yes] {
  block ^entry:
    call void @touch(%a)
    %v = load i32 %a
    %w = load i32 %b
    %x = load i32 %c
    %y = load i32 %d
    %z = load i32 %e
    %s1 = binary add i32 %v, %w
    %s2 = binary add i32 %s1, %x
    %s3 = binary add i32 %s2, %y
    %s4 = binary add i32 %s3, %z
    return i32 %s4
}

function @main() -> i32 [role=entry] {
  slot $v : obj<20x4>

  block ^entry:
    %p0 = addr $v
    store i32 1, %p0
    %p1 = index i32 %p0, 1
    store i32 2, %p1
    %p2 = index i32 %p0, 2
    store i32 3, %p2
    %p3 = index i32 %p0, 3
    store i32 4, %p3
    %p4 = index i32 %p0, 4
    store i32 5, %p4
    %r = call i32 @three(%p0, %p1, %p2)
    %q = call i32 @five(%p0, %p1, %p2, %p3, %p4)
    %t = binary add i32 %r, %q
    return i32 %t
}

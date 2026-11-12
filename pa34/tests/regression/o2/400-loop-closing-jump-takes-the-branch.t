# A loop block that ends `jcc exit; jmp itself`, laid out after the code it
# exits to, where the exit is the more frequent block by estimate (every
# path reaches it; the loop only after two tests of memory against zero).
# The jump closes the loop the branch leaves, so the two change places: the
# conditional branch repeats the loop and the jump runs once, on the way out.
global @text : obj<8x1> [binding=internal] = {
  u8 97
  u8 98
  u8 99
  u8 100
  u8 0
  zero 3
}
global @found : obj<16x8> [binding=internal] = {
  zero 16
}

function @scan(%p : ptr, %out : ptr) -> void [binding=internal, no_inline=yes] {
  block ^entry:
    %c0 = load typed u8 %p
    %z0 = cmp eq u8 %c0, 0
    branch %z0, ^done, ^second

  block ^second:
    %p1 = index i8 %p, 1
    %c1 = load typed u8 %p1
    %z1 = cmp eq u8 %c1, 0
    branch %z1, ^done, ^loop

  block ^loop:
    %q = phi ptr [^second: %p1, ^loop: %q1]
    %q1 = index i8 %q, 1
    %c = load typed u8 %q1
    %z = cmp eq u8 %c, 0
    branch %z, ^done, ^loop

  block ^done:
    %e = phi ptr [^entry: %p, ^second: %p1, ^loop: %q1]
    %d = binary sub ptr %e, %p
    %n = binary add i64 %d, 0
    store typed i64 %n, %out
    %last = load typed u8 %e
    %wide = convert zext i64 u8 %last
    %slot = index i8 %out, 8
    store typed i64 %wide, %slot
    return void
}

function @main() -> i32 [role=entry] {
  block ^entry:
    call void @scan(@text, @found)
    %n = load typed i64 @found
    %r = convert trunc i32 i64 %n
    return i32 %r
}

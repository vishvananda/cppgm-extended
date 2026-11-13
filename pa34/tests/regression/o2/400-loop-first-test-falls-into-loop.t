# A loop's first test, left on the entry path once the test's copy closes the
# loop, enters the loop or goes around it. The loop is the likelier side
# whatever the test reads (here one bit of a table byte, which a branch
# elsewhere mostly finds clear): the loop follows the test and falls into the
# code after it. Laid out after the function's other code, each run of the
# loop would take a branch into it and a jump back out.
global @classes : obj<4x1> [binding=internal] = {
  u8 0
  u8 1
  u8 3
  u8 0
}
global @text : obj<8x1> [binding=internal] = {
  u8 1
  u8 2
  u8 1
  u8 0
  zero 4
}
global @ends : obj<8x4> [binding=internal] = {
  zero 8
}

function @take(%bytes : ptr, %at : u32, %out : ptr) -> u8 [binding=internal, no_inline=yes] {
  block ^entry:
    jump ^test

  block ^test:
    %p = phi u32 [^entry: %at, ^step: %next]
    %byte_at = index u8 %bytes, %p
    %byte = load u8 %byte_at
    %class_at = index u8 @classes, %byte
    %class = load u8 %class_at
    %wide = convert zext i32 u8 %class
    %bit = binary and i32 %wide, 1
    branch %bit, ^step, ^after

  block ^step:
    %next = binary add u32 %p, 1
    jump ^test

  block ^after:
    %none = cmp eq u32 %at, %p
    branch %none, ^empty, ^some

  block ^empty:
    return u8 0

  block ^some:
    store typed u32 %p, %out
    %second = index i8 %out, 4
    store typed u32 %p, %second
    return u8 1
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %taken = call u8 @take(@text, 0, @ends)
    %end = load typed u32 @ends
    %signed = copy i32 %end
    %wide = convert zext i32 u8 %taken
    %scaled = binary mul i32 %wide, 10
    %result = binary add i32 %scaled, %signed
    return i32 %result
}

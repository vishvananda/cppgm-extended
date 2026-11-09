# A global byte table and a narrow masked index model a tagged pointer walk.
# The sum used only by the load can share its base/index operand. A second
# function keeps the computed address live across the load as a control.
global @offsets = {
  i8 16
  i8 24
  i8 16
  i8 24
  i8 16
  i8 24
  i8 16
  i8 24
}

function @walk(%p : ptr) -> ptr [no_inline=yes] {
  block ^entry:
    %tag = load u16 %p
    %key = binary and u16 %tag, 7
    %at = index i8 @offsets, %key
    %offset = load i8 %at
    %field = index i8 %p, %offset
    %next = load ptr %field
    return ptr %next
}

function @live_address(%key : i64, %out : ptr) -> ptr [no_inline=yes] {
  block ^entry:
    %at = index i8 @offsets, %key
    %value = load i8 %at
    store i8 %value, %out
    return ptr %at
}

function @masked16(%x : u16) -> i32 [no_inline=yes] {
  block ^entry:
    %mask = binary and u16 %x, 127
    %yes = cmp ne u16 %mask, 0
    %wide = convert zext i32 u16 %mask
    %result = select i32 %yes, %wide, 1000
    return i32 %result
}

function @masked8(%x : u8) -> i32 [no_inline=yes] {
  block ^entry:
    %mask = binary and u8 %x, 63
    %yes = cmp ne u8 %mask, 0
    %wide = convert zext i32 u8 %mask
    %result = select i32 %yes, %wide, 1000
    return i32 %result
}

# The sign differs between an AND at machine width and a narrow test when
# the mask contains the narrow sign bit. These signed conditions must stay.
function @signed16(%x : i16) -> i32 [no_inline=yes] {
  block ^entry:
    %mask = binary and i16 %x, 32768
    %negative = cmp lt i16 %mask, 0
    %wide = convert sext i32 i16 %mask
    %result = select i32 %negative, %wide, 1000
    return i32 %result
}

function @signed8(%x : i8) -> i32 [no_inline=yes] {
  block ^entry:
    %mask = binary and i8 %x, 128
    %negative = cmp lt i8 %mask, 0
    %wide = convert sext i32 i8 %mask
    %result = select i32 %negative, %wide, 1000
    return i32 %result
}

function @main() -> i32 [role=entry] {
  slot $node : obj<32x8>
  slot $out : i8
  block ^entry:
    %p = addr $node
    %a = index i8 %p, 16
    %b = index i8 %p, 24
    store ptr %a, %a
    store ptr %b, %b
    store u16 6, %p
    %r0 = call ptr @walk(%p)
    %ok0 = cmp eq ptr %r0, %a
    store u16 7, %p
    %r1 = call ptr @walk(%p)
    %ok1 = cmp eq ptr %r1, %b
    %out = addr $out
    %at = call ptr @live_address(3, %out)
    %expected = index i8 @offsets, 3
    %ok2 = cmp eq ptr %at, %expected
    %v = load i8 %out
    %ok3 = cmp eq i8 %v, 24
    %r2 = call i32 @masked16(65535)
    %ok4 = cmp eq i32 %r2, 127
    %r3 = call i32 @masked8(255)
    %ok5 = cmp eq i32 %r3, 63
    %r4 = call i32 @masked16(128)
    %ok6 = cmp eq i32 %r4, 1000
    %r5 = call i32 @masked8(128)
    %ok7 = cmp eq i32 %r5, 1000
    %r6 = call i32 @signed16(-1)
    %ok8 = cmp eq i32 %r6, -32768
    %r7 = call i32 @signed8(-1)
    %ok9 = cmp eq i32 %r7, -128
    %r8 = call i32 @signed16(1)
    %ok10 = cmp eq i32 %r8, 1000
    %r9 = call i32 @signed8(1)
    %ok11 = cmp eq i32 %r9, 1000
    %a0 = binary and i64 %ok0, %ok1
    %a1 = binary and i64 %a0, %ok2
    %a2 = binary and i64 %a1, %ok3
    %a3 = binary and i64 %a2, %ok4
    %a4 = binary and i64 %a3, %ok5
    %a5 = binary and i64 %a4, %ok6
    %a6 = binary and i64 %a5, %ok7
    %a7 = binary and i64 %a6, %ok8
    %a8 = binary and i64 %a7, %ok9
    %a9 = binary and i64 %a8, %ok10
    %a10 = binary and i64 %a9, %ok11
    %failed = cmp eq i64 %a10, 0
    %status = convert trunc i32 i64 %failed
    return i32 %status
}

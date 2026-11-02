# The target of a call through a memory cell is read by the call itself, past
# the move of its argument.
function @one(%x : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %r = binary add i64 %x, 1
    return i64 %r
}

function @through(%x : i64, %cell : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %f = load typed ptr %cell
    %r = call i64 %f(%x) as (%a : i64) -> i64
    return i64 %r
}

function @main() -> i32 [role=entry] {
  slot $cell : obj<8x8>
  block ^entry:
    %c = addr $cell
    store typed ptr @one, %c
    %r = call i64 @through(41, %c)
    %t = convert trunc i32 i64 %r
    return i32 %t
}

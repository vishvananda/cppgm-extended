# A load runs on every entry of a rotated loop: it leaves through the chain of
# steps that names its address, however many values already live across the loop.

function @strides(%e : ptr, %d : ptr, %k : i64, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %base = load typed ptr %e
    %row = index i8 %base, %k
    %field = index i8 %row, 24
    %s = load typed i32 %field
    %w = convert sext i64 i32 %s
    %m = binary mul i64 %w, %i
    %p = index f64 %d, %m
    store typed f64 1, %p
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# A store of the load's type may name the same memory.
function @same_type(%e : ptr, %d : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %f = index i8 %e, 24
    %s = load typed i32 %f
    %p = index i32 %d, %i
    store typed i32 %s, %p
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

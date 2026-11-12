# An effect-free loop over a pointer goes: the pointer read past it takes its
# exit value, an index from its start, in the preheader.
global @values : obj<32x8> = { i64 1 i64 2 i64 3 i64 4 }

function @end_of(%p : ptr, %n : i64) -> ptr [no_inline=yes] {
  block ^entry:
    jump ^header

  block ^header:
    %i = phi i64 [^entry: 0, ^body: %next]
    %q = phi ptr [^entry: %p, ^body: %step]
    %more = cmp lt i64 %i, %n
    branch %more, ^body, ^exit

  block ^body:
    %step = index i64 %q, 1
    %next = binary add i64 %i, 1
    jump ^header

  block ^exit:
    return ptr %q
}

function @main() -> i32 {
  block ^entry:
    %end = call ptr @end_of(@values, 3)
    %last = load i64 %end
    %none = call ptr @end_of(@values, 0)
    %first = load i64 %none
    %last_ok = cmp eq i64 %last, 4
    %first_ok = cmp eq i64 %first, 1
    %last_bit = select i32 %last_ok, 0, 1
    %first_bit = select i32 %first_ok, 0, 2
    %status = binary or i32 %last_bit, %first_bit
    return i32 %status
}

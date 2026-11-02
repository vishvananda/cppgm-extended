# Partial inlining copies a callee up to the block that calls, and the copy
# calls the callee from its entry there. A walk that ends in a call stays out
# of its callers: the copy would make the call walk the list again.
global @head : obj<16x8> = { zero 16 }
global @made : i64 = 0

declare function @note(%n : i64) -> void [linkage=c, binding=strong, object=note]

function @find(%n : i64) -> i64 {
  block ^entry:
    jump ^walk
  block ^walk:
    %p = phi ptr [^entry: @head, ^next: %q]
    %v = load typed i64 %p
    %same = cmp eq i64 %v, %n
    branch %same, ^found, ^next
  block ^next:
    %f = index i8 %p, 8
    %q = load typed ptr %f
    %end = cmp eq ptr %q, 0
    branch %end, ^missing, ^walk
  block ^found:
    return i64 1
  block ^missing:
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    call void @note(%n)
    return i64 0
}

function @twice(%n : i64) -> i64 {
  block ^entry:
    %a = call i64 @find(%n)
    %m = binary add i64 %n, 1
    %b = call i64 @find(%m)
    %r = binary add i64 %a, %b
    return i64 %r
}

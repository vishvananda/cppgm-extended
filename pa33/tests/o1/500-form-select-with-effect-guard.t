# The pure diamond may become a select at O1. A conditional volatile read
# must remain conditional, including when its pointer is null on the other arm.
function @choose(%condition : i1, %a : i64, %b : i64) -> i64 [no_inline=yes] {
  block ^entry:
    branch %condition, ^left, ^right
  block ^left:
    %x = binary add i64 %a, 3
    jump ^join
  block ^right:
    %y = binary sub i64 %b, 5
    jump ^join
  block ^join:
    %value = phi i64 [^left: %x, ^right: %y]
    return i64 %value
}

function @guarded(%condition : i1, %pointer : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    branch %condition, ^read, ^skip
  block ^read:
    %value = load volatile i64 %pointer
    jump ^join
  block ^skip:
    jump ^join
  block ^join:
    %result = phi i64 [^read: %value, ^skip: 7]
    return i64 %result
}

function @main() -> i32 [role=entry] {
  slot $storage : i64
  block ^entry:
    store i64 19, $storage
    %pointer = addr $storage
    %left = call i64 @choose(1, 11, 23)
    %right = call i64 @choose(0, 11, 23)
    %read = call i64 @guarded(1, %pointer)
    %skip = call i64 @guarded(0, nullptr)
    %a = cmp eq i64 %left, 14
    %b = cmp eq i64 %right, 18
    %c = cmp eq i64 %read, 19
    %d = cmp eq i64 %skip, 7
    %ab = binary and i1 %a, %b
    %cd = binary and i1 %c, %d
    %all = binary and i1 %ab, %cd
    branch %all, ^pass, ^fail
  block ^pass:
    return i32 0
  block ^fail:
    return i32 1
}

# The sum of products is only stored, and the remainder only tested: nothing
# reads either above 32 bits, so neither is extended. The running count is read
# as an index and stays extended.
function @sum_products(%p : ptr, %n : i32, %out : ptr) -> i32 [no_inline=yes] {
  block ^entry:
    %has = cmp gt i32 %n, 0
    branch %has, ^loop, ^done

  block ^loop:
    %i = phi i32 [^entry: 0, ^loop: %i2]
    %s = phi i32 [^entry: 0, ^loop: %s2]
    %at = index i32 %p, %i
    %v = load typed i32 %at
    %m = binary mul nowrap i32 %v, %v
    %s2 = binary add nowrap i32 %s, %m
    %i2 = binary add nowrap i32 %i, 1
    %c = cmp lt i32 %i2, %n
    branch %c, ^loop, ^end

  block ^end:
    %t = phi i32 [^loop: %s2]
    store typed i32 %t, %out
    jump ^done

  block ^done:
    return i32 %n
}

function @divides(%a : i32, %b : i32) -> i32 [no_inline=yes] {
  block ^entry:
    %r = binary mod i32 %a, %b
    branch %r, ^no, ^yes

  block ^yes:
    return i32 1

  block ^no:
    return i32 0
}

global @table : obj<16x4> = { zero 16 }

function @main() -> i32 [role=entry] {
  block ^entry:
    %t = addr @table
    %a = index i32 %t, 0
    store typed i32 3, %a
    %b = index i32 %t, 1
    store typed i32 4, %b
    %c = index i32 %t, 2
    store typed i32 5, %c
    %o = index i32 %t, 3
    %n = call i32 @sum_products(%t, 3, %o)
    %s = load typed i32 %o
    %d = call i32 @divides(9, 3)
    %e = call i32 @divides(9, 4)
    %x = binary mul i32 %s, 2
    %y = binary add i32 %x, %d
    %z = binary sub i32 %y, %e
    return i32 %z
}

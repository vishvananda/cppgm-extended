# Jump threading threads the tests of the phis that ^land_end and ^merge
# make. It demotes a phi it threads through to a slot and reloads it in the
# phi's place. The reload is a temporary of this run, which has no use count.
# A later branch of the same run tests the reload: it is not paired with it
# as with a compare. (A bounds-checked build of the optimizer read past the
# use counts on this input.)
function @f(%a : u32, %b : u32, %c : u32, %d : u32, %p : ptr) -> void [no_inline=yes] {
  block ^entry:
    %or1 = binary or u32 %a, %b
    %or2 = binary or u32 %or1, %c
    %or3 = binary or u32 %or2, %d
    %big = cmp ugt u32 %or3, 16777215
    branch %big, ^short, ^rhs

  block ^rhs:
    %c2 = cmp ugt u32 %c, 65535
    branch %c2, ^rhs2, ^land_short

  block ^rhs2:
    %c3 = cmp ugt u32 %d, 65535
    jump ^land_end

  block ^land_short:
    jump ^land_end

  block ^land_end:
    %l = phi i64 [^rhs2: %c3, ^land_short: 0]
    jump ^merge

  block ^short:
    jump ^merge

  block ^merge:
    %use = phi i64 [^land_end: %l, ^short: 1]
    %u8 = convert trunc u8 i64 %use
    jump ^loop

  block ^loop:
    %k = phi i32 [^merge: 0, ^latch: %k1]
    %more = cmp lt i32 %k, 4
    branch %more, ^body, ^exit

  block ^body:
    branch %u8, ^then, ^latch

  block ^then:
    %s = binary ushr u32 %a, 24
    %t = convert trunc u8 u32 %s
    store u8 %t, %p
    jump ^latch

  block ^latch:
    %k1 = binary add nowrap i32 %k, 1
    jump ^loop

  block ^exit:
    return void
}

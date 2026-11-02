# A dividend that dies at a constant divide stays out of rax: the magic
# multiply writes rax and then reads the dividend again for the sign and the
# remainder. (Only the hardware divide, which reads the dividend in rax first,
# may have it computed there.) Here the dividend is an add the loop computes
# from a phi, which the allocator would have placed in rax.
global @last : i32 = 42

function @main() -> i32 [role=entry] {
  block ^entry:
    %start = load i32 @last
    jump ^loop

  block ^loop:
    %x = phi i32 [^entry: %start, ^loop: %z]
    %i = phi i32 [^entry: 0, ^loop: %j]
    %m = binary mul nowrap i32 %x, 3877
    %y = binary add nowrap i32 %m, 29573
    %z = binary mod i32 %y, 139968
    store i32 %z, @last
    %j = binary add i32 %i, 1
    %more = cmp lt i32 %j, 5
    branch %more, ^loop, ^done

  block ^done:
    %r = binary and i32 %z, 255
    return i32 %r
}

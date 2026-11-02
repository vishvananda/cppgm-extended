# `sqrt` of what may be negative: the instruction always, and the call only on
# the negative path, for `errno`. A loop around it keeps its loads outside: the
# call writes `errno`, which a read of another type cannot name.
declare function @sqrt(%x : f64) -> f64 [linkage=c, binding=strong, object=sqrt]
declare function @sqrtf(%x : f32) -> f32 [linkage=c, binding=strong, object=sqrtf]

function @one(%x : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %r = call f64 @sqrt(%x)
    %s = binary add f64 %r, 1
    return f64 %s
}

function @narrow(%x : f32) -> f32 [no_inline=yes] {
  block ^entry:
    %r = call f32 @sqrtf(%x)
    return f32 %r
}

function @two(%x : f64, %y : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %r = call f64 @sqrt(%x)
    %t = call f64 @sqrt(%y)
    %s = binary add f64 %r, %t
    return f64 %s
}

function @square(%x : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %m = binary mul f64 %x, %x
    %r = call f64 @sqrt(%m)
    return f64 %r
}

function @in_loop(%p : ptr, %q : ptr, %n : i64) -> f64 [no_inline=yes] {
  block ^entry:
    jump ^loop

  block ^loop:
    %i = phi i64 [^entry: 0, ^loop: %next]
    %sum = phi f64 [^entry: 0, ^loop: %more]
    %scale = load typed f64 %q
    %at = index f64 %p, %i
    %v = load typed f64 %at
    %m = binary mul f64 %v, %scale
    %r = call f64 @sqrt(%m)
    %more = binary add f64 %sum, %r
    %next = binary add nowrap i64 %i, 1
    %again = cmp lt i64 %next, %n
    branch %again, ^loop, ^done

  block ^done:
    return f64 %more
}

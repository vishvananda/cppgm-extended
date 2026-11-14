# A library mathematics call and an allocation call write `errno` and nothing
# else the program reaches. `errno` is an object of its own: it is no part of
# a larger object. A parameter with a known object larger than an `int`
# therefore keeps its `int` fields across such a call: a loop around the call
# reads the field once, before it, and a read after an allocation reuses the
# value read before it. An `int` object of its own behind a parameter may be
# `errno`: its reads stay where they are.
declare function @sqrt(%x : f64) -> f64 [linkage=c, binding=strong, object=sqrt]
declare function @malloc(%bytes : i64) -> ptr [unwind=no, linkage=c, binding=strong, object=malloc]

function @field_in_loop(%box : ptr [object_bytes=16], %p : ptr, %n : i64) -> f64 [binding=strong, no_inline=yes] {
  block ^entry:
    jump ^loop

  block ^loop:
    %i = phi i64 [^entry: 0, ^loop: %next]
    %sum = phi f64 [^entry: 0, ^loop: %more]
    %field = index inbounds i8 [projection=field] %box, 8
    %scale = load typed i32 %field
    %factor = convert sitofp f64 i32 %scale
    %at = index f64 %p, %i
    %v = load typed f64 %at
    %m = binary mul f64 %v, %factor
    %r = call f64 @sqrt(%m)
    %more = binary add f64 %sum, %r
    %next = binary add nowrap i64 %i, 1
    %again = cmp lt i64 %next, %n
    branch %again, ^loop, ^done

  block ^done:
    return f64 %more
}

function @int_in_loop(%cell : ptr [object_bytes=4], %p : ptr, %n : i64) -> f64 [binding=strong, no_inline=yes] {
  block ^entry:
    jump ^loop

  block ^loop:
    %i = phi i64 [^entry: 0, ^loop: %next]
    %sum = phi f64 [^entry: 0, ^loop: %more]
    %scale = load typed i32 %cell
    %factor = convert sitofp f64 i32 %scale
    %at = index f64 %p, %i
    %v = load typed f64 %at
    %m = binary mul f64 %v, %factor
    %r = call f64 @sqrt(%m)
    %more = binary add f64 %sum, %r
    %next = binary add nowrap i64 %i, 1
    %again = cmp lt i64 %next, %n
    branch %again, ^loop, ^done

  block ^done:
    return f64 %more
}

function @field_past_allocation(%box : ptr [object_bytes=16], %out : ptr) -> i32 [binding=strong, no_inline=yes] {
  block ^entry:
    %field = index inbounds i8 [projection=field] %box, 8
    %before = load typed i32 %field
    %fresh = call ptr @malloc(8)
    store typed ptr %fresh, %out
    %after = load typed i32 %field
    %sum = binary add i32 %before, %after
    return i32 %sum
}

function @int_past_allocation(%cell : ptr [object_bytes=4], %out : ptr) -> i32 [binding=strong, no_inline=yes] {
  block ^entry:
    %before = load typed i32 %cell
    %fresh = call ptr @malloc(8)
    store typed ptr %fresh, %out
    %after = load typed i32 %cell
    %sum = binary add i32 %before, %after
    return i32 %sum
}

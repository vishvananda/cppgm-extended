# In a loop with calls on some paths only, a location the loop loads lives
# in a register: loaded before the loop and again after each call. Its
# stores stay where they are. Here @seed's load leaves the generator's chain,
# and so does the bound's load, which the loop never stores. @flush writes
# @seed, so the value after its call is the one it wrote. In @every the call
# runs on every pass, and the loads stay.

global @seed : i32 = 42
global @bound : i32 = 9
global @flushes : i32 = 0

function @flush() -> void [no_inline=yes] {
  block ^entry:
    %f = load i32 @flushes
    %g = binary add i32 %f, 1
    store i32 %g, @flushes
    %s = load i32 @seed
    %t = binary add i32 %s, 1000
    store i32 %t, @seed
    return void
}

function @generate() -> i32 [no_inline=yes] {
  block ^entry:
    jump ^head

  block ^head:
    %i = phi i32 [^entry: 0, ^next: %j, ^flushed: %j]
    %sum = phi i32 [^entry: 0, ^next: %more, ^flushed: %more]
    %s = load i32 @seed
    %m = binary mul i32 %s, 3877
    %a = binary add i32 %m, 29573
    %z = binary mod i32 %a, 139968
    store i32 %z, @seed
    %bits = binary and i32 %z, 15
    %more = binary add i32 %sum, %bits
    %j = binary add i32 %i, 1
    %every = binary and i32 %j, 3
    %full = cmp eq i32 %every, 0
    branch %full, ^flush, ^next

  block ^flush:
    call void @flush()
    jump ^flushed

  block ^flushed:
    %n = load i32 @bound
    %go = cmp lt i32 %j, %n
    branch %go, ^head, ^done

  block ^next:
    %b = load i32 @bound
    %on = cmp lt i32 %j, %b
    branch %on, ^head, ^done

  block ^done:
    return i32 %more
}

function @every(%n : i32) -> i32 [binding=strong, no_inline=yes] {
  block ^entry:
    jump ^head

  block ^head:
    %i = phi i32 [^entry: 0, ^head: %j]
    %s = load i32 @seed
    %t = binary add i32 %s, 1
    store i32 %t, @seed
    call void @flush()
    %j = binary add i32 %i, 1
    %go = cmp lt i32 %j, %n
    branch %go, ^head, ^done

  block ^done:
    %r = load i32 @seed
    return i32 %r
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %g = call i32 @generate()
    %e = call i32 @every(3)
    %f = load i32 @flushes
    %h = binary and i32 %e, 255
    %x = binary add i32 %g, %h
    %y = binary mul i32 %f, 1000
    %r = binary add i32 %x, %y
    %b = binary and i32 %r, 255
    return i32 %b
}

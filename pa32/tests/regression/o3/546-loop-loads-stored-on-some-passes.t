# A call-free loop that stores no location on every pass keeps a location
# it loads in a register too. Here @best's load leaves the loop: the register
# takes the value of the store on the passes that store it, and the store
# stays where it is. @limit, which the loop never stores, also leaves.

global @best : i32 = 0
global @limit : i32 = 40

function @scan() -> i32 [no_inline=yes] {
  block ^entry:
    jump ^head

  block ^head:
    %i = phi i32 [^entry: 0, ^keep: %j, ^raise: %j]
    %count = phi i32 [^entry: 0, ^keep: %count, ^raise: %more]
    %m = binary mul i32 %i, 7919
    %a = binary add i32 %m, 13
    %v = binary mod i32 %a, 1000
    %b = load i32 @best
    %j = binary add i32 %i, 1
    %up = cmp gt i32 %v, %b
    branch %up, ^raise, ^keep

  block ^raise:
    store i32 %v, @best
    %more = binary add i32 %count, 1
    %n = load i32 @limit
    %go = cmp lt i32 %j, %n
    branch %go, ^head, ^done

  block ^keep:
    %k = load i32 @limit
    %on = cmp lt i32 %j, %k
    branch %on, ^head, ^done

  block ^done:
    %c = phi i32 [^raise: %more, ^keep: %count]
    return i32 %c
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %c = call i32 @scan()
    %b = load i32 @best
    %s = binary mul i32 %c, 1000
    %r = binary add i32 %s, %b
    %w = binary mod i32 %r, 251
    return i32 %w
}

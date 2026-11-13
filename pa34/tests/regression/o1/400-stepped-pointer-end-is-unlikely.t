# A pointer stepped by a constant in a loop reaches the end of its segment
# once per segment. Placement puts the unequal side after the test and
# joins it with the rest of the loop; the side that moves to the next
# segment goes out of line and jumps back.

function @same(%map : ptr, %flat : ptr, %n : i64) -> i32 [no_inline=yes] {
  block ^entry:
    %first = load typed ptr %map
    %end0 = index i32 %first, 4
    %pend = index i32 %flat, %n
    %empty = cmp eq ptr %flat, %pend
    branch %empty, ^equal, ^check
  block ^check:
    %cur = phi ptr [^entry: %first, ^step: %curj]
    %end = phi ptr [^entry: %end0, ^step: %endj]
    %node = phi ptr [^entry: %map, ^step: %nodej]
    %p = phi ptr [^entry: %flat, ^step: %p1]
    %a = load typed i32 %cur
    %b = load typed i32 %p
    %differ = cmp ne i32 %a, %b
    branch %differ, ^unequal, ^next
  block ^next:
    %cur1 = index i32 %cur, 1
    %full = cmp eq ptr %cur1, %end
    branch %full, ^advance, ^step
  block ^advance:
    %node1 = index ptr %node, 1
    %seg = load typed ptr %node1
    %end1 = index i32 %seg, 4
    jump ^step
  block ^step:
    %curj = phi ptr [^next: %cur1, ^advance: %seg]
    %endj = phi ptr [^next: %end, ^advance: %end1]
    %nodej = phi ptr [^next: %node, ^advance: %node1]
    %p1 = index i32 %p, 1
    %last = cmp eq ptr %p1, %pend
    branch %last, ^equal, ^check
  block ^equal:
    return i32 1
  block ^unequal:
    return i32 0
}

function @main() -> i32 [role=entry] {
  slot $data : obj<48x4>
  slot $flat : obj<48x4>
  slot $map : obj<32x8>
  block ^entry:
    %d = addr $data
    %f = addr $flat
    %m = addr $map
    jump ^fill
  block ^fill:
    %k = phi i64 [^entry: 0, ^fill: %k1]
    %triple = binary mul i64 %k, 3
    %val = convert trunc i32 i64 %triple
    %slot = index i32 %d, %k
    store typed i32 %val, %slot
    %copy = index i32 %f, %k
    store typed i32 %val, %copy
    %k1 = binary add i64 %k, 1
    %again = cmp ult i64 %k1, 12
    branch %again, ^fill, ^link
  block ^link:
    store typed ptr %d, %m
    %s1 = index i32 %d, 4
    %m1 = index ptr %m, 1
    store typed ptr %s1, %m1
    %s2 = index i32 %d, 8
    %m2 = index ptr %m, 2
    store typed ptr %s2, %m2
    %m3 = index ptr %m, 3
    store typed ptr nullptr, %m3
    %equal = call i32 @same(%m, %f, 12)
    %ninth = index i32 %f, 9
    store typed i32 5, %ninth
    %changed = call i32 @same(%m, %f, 12)
    %twice = binary mul i32 %equal, 2
    %code = binary add i32 %twice, %changed
    %ok = cmp eq i32 %code, 2
    %r = select i32 %ok, 0, 1
    return i32 %r
}

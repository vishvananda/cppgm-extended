# A latch can bound a search even when the header leaves early. Each probe
# must still run only if earlier ones failed; no access may pass the bound.
global @values : obj<40x8> = {
  i64 2
  i64 4
  i64 6
  i64 8
  i64 10
}
global @probes : i64 = 0

function @search(%wanted : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %base = addr @values
    %end = index i64 %base, 4
    jump ^head
  block ^head:
    %p = phi ptr [^entry: %base, ^latch: %next]
    %count = load volatile i64 @probes
    %count_next = binary add i64 %count, 1
    store volatile i64 %count_next, @probes
    %x = load i64 %p
    %found = cmp eq i64 %x, %wanted
    branch %found, ^done, ^latch
  block ^latch:
    %next = index i64 %p, 1
    %more = cmp ne ptr %next, %end
    branch %more, ^head, ^done
  block ^done:
    %result = phi ptr [^head: %p, ^latch: %next]
    %bytes = binary sub ptr %result, %base
    return i64 %bytes
}

function @backward(%wanted : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %base = addr @values
    %begin = index i64 %base, 4
    jump ^head
  block ^head:
    %p = phi ptr [^entry: %begin, ^latch: %next]
    %x = load i64 %p
    %found = cmp eq i64 %x, %wanted
    branch %found, ^done, ^latch
  block ^latch:
    %next = index i64 %p, -1
    %more = cmp eq ptr %base, %next
    branch %more, ^done, ^head
  block ^done:
    %result = phi ptr [^head: %p, ^latch: %next]
    %bytes = binary sub ptr %result, %base
    return i64 %bytes
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call i64 @search(2)
    %count_a = load volatile i64 @probes
    %b = call i64 @search(8)
    %count_b = load volatile i64 @probes
    %c = call i64 @search(10)
    %count_c = load volatile i64 @probes
    %d = call i64 @backward(10)
    %e = call i64 @backward(4)
    %f = call i64 @backward(2)
    %aa = cmp eq i64 %a, 0
    %bb = cmp eq i64 %b, 24
    %cc = cmp eq i64 %c, 32
    %dd = cmp eq i64 %d, 32
    %ee = cmp eq i64 %e, 8
    %ff = cmp eq i64 %f, 0
    %ca = cmp eq i64 %count_a, 1
    %cb = cmp eq i64 %count_b, 5
    %ccount = cmp eq i64 %count_c, 9
    %ab = binary and i1 %aa, %bb
    %cd = binary and i1 %cc, %dd
    %ef = binary and i1 %ee, %ff
    %abcd = binary and i1 %ab, %cd
    %indices = binary and i1 %abcd, %ef
    %cab = binary and i1 %ca, %cb
    %counts = binary and i1 %cab, %ccount
    %ok = binary and i1 %indices, %counts
    %r = select i32 %ok, 0, 1
    return i32 %r
}

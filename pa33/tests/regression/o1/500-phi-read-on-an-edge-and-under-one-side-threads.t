# A merge holds a value phi and a flag phi and branches on the flag. The
# value is read past the merge in two ways: by the exit's phi, along the
# merge's own edge, and under the other successor, which only the merge
# enters (the loop carries it to the next pass). The read along the edge
# does not hold a predecessor back: the block that sets the flag jumps to
# the exit and brings its own value. The other successor still follows the
# merge, so its reads stay.
global @data : obj<48x8> = { i64 8 i64 4 i64 6 i64 7 i64 2 i64 9 }

function @scan(%from : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    jump ^head

  block ^head:
    %i = phi i64 [^entry: %from, ^next: %i1]
    %last = phi i64 [^entry: 100, ^next: %value]
    %more = cmp lt i64 %i, 6
    branch %more, ^body, ^end

  block ^body:
    %base = addr @data
    %at = index inbounds i64 [projection=array_element] %base, %i
    %x = load i64 %at
    %odd = binary and i64 %x, 1
    branch %odd, ^found, ^skip

  block ^found:
    %hit = binary add i64 %x, %last
    jump ^merge

  block ^skip:
    %sum = binary add i64 %last, %x
    jump ^merge

  block ^merge:
    %value = phi i64 [^found: %hit, ^skip: %sum]
    %holds = phi i64 [^found: 1, ^skip: 0]
    branch %holds, ^done, ^next

  block ^next:
    %i1 = binary add i64 %i, 1
    jump ^head

  block ^end:
    jump ^done

  block ^done:
    %r = phi i64 [^merge: %value, ^end: 0]
    return i64 %r
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call i64 @scan(0)
    %b = call i64 @scan(4)
    %s = binary add i64 %a, %b
    %m = binary umod i64 %s, 251
    %t = convert trunc i32 i64 %m
    return i32 %t
}

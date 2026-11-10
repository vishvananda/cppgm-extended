# At -O2 a callee whose handler only cleans up and resumes goes inline into
# a caller that catches nothing: @guarded goes into @middle's loop (the
# ordinary wave takes such a callee at loop sites alone), and its record
# nests in @middle's frame. A resume continues to @main's handler. @main
# reads the exception, so @middle, which now holds the record, stays a call.
global @cleanups : i64 = 0

function @raise(%n : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %big = cmp ugt i64 %n, 3
    branch %big, ^throw, ^ok

  block ^throw:
    throw i64 %n

  block ^ok:
    %r = binary add i64 %n, 10
    return i64 %r
}

function @guarded(%n : i64) -> i64 [binding=internal] {
  block ^entry:
    eh_try ^cleanup
    %r = call i64 @raise(%n)
    eh_end
    return i64 %r

  block ^cleanup:
    %c = load i64 @cleanups
    %d = binary add i64 %c, 1
    store i64 %d, @cleanups
    resume
}

function @middle(%n : i64) -> i64 [binding=internal] {
  block ^entry:
    jump ^loop

  block ^loop:
    %i = phi i64 [^entry: 0, ^loop: %next]
    %sum = phi i64 [^entry: 0, ^loop: %total]
    %a = call i64 @guarded(%i)
    %total = binary add i64 %sum, %a
    %next = binary add i64 %i, 1
    %more = cmp ult i64 %next, %n
    branch %more, ^loop, ^done

  block ^done:
    return i64 %total
}

function @main() -> i64 [role=entry] {
  block ^entry:
    eh_try ^handler
    %x = call i64 @middle(3)
    %y = call i64 @middle(5)
    eh_end
    %z = binary add i64 %x, %y
    return i64 %z

  block ^handler:
    %e = exception i64
    %c = load i64 @cleanups
    %s = binary mul i64 %c, 100
    %t = binary add i64 %s, %e
    return i64 %t
}

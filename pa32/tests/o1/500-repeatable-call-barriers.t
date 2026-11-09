# Memory effects alone do not promise equal results. Reuse only explicitly
# repeatable calls, keep their first execution, and stop at opaque calls.
declare function @memory_only(%p : ptr) -> i64 [effects=readonly, unwind=no]
declare function @repeatable(%p : ptr) -> i64 [effects=readonly, repeatable=yes]

function @unmarked(%p : ptr) -> i64 {
  block ^entry:
    %a = call i64 @memory_only(%p)
    %b = call i64 @memory_only(%p)
    %sum = binary add i64 %a, %b
    return i64 %sum
}

function @reuse(%p : ptr) -> i64 {
  block ^entry:
    %a = call i64 @repeatable(%p)
    %b = call i64 @repeatable(%p)
    %sum = binary add i64 %a, %b
    return i64 %sum
}

function @opaque_between(%p : ptr) -> i64 {
  block ^entry:
    %a = call i64 @repeatable(%p)
    %opaque = call i64 @memory_only(%p)
    %b = call i64 @repeatable(%p)
    %sum = binary add i64 %a, %b
    %result = binary add i64 %sum, %opaque
    return i64 %result
}

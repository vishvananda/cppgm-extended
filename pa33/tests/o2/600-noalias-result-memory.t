declare function @malloc(%n : i64) -> ptr [linkage=c, binding=strong, object=malloc, unwind=no]
global @published : ptr = 0

function @fresh(%p : ptr) -> ptr [noalias_result=yes, no_inline=yes] {
  block ^entry:
    store i64 99, %p
    %q = call ptr @malloc(16)
    return ptr %q
}
function @ordinary(%p : ptr) -> ptr [no_inline=yes] {
  block ^entry:
    return ptr %p
}

# Return aliasing does not say the call leaves existing memory unchanged.
function @call_writes(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %before = load i64 %p
    %q = call ptr @fresh(%p)
    %after = load i64 %p
    %sum = binary add i64 %before, %after
    return i64 %sum
}

# The incoming pointer cannot reach the returned storage.
function @fresh_store(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %q = call ptr @fresh(%p)
    %before = load i64 %p
    store i64 7, %q
    %after = load i64 %p
    %sum = binary add i64 %before, %after
    return i64 %sum
}

# The same fact applies to a typed virtual/indirect call boundary.
function @indirect_fresh(%fn : ptr, %p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %q = call ptr %fn(%p) as (%arg : ptr) -> ptr [noalias_result=yes]
    %before = load i64 %p
    store i64 7, %q
    %after = load i64 %p
    %sum = binary add i64 %before, %after
    return i64 %sum
}

# No promise on this return: its pointer can equal the incoming pointer.
function @ordinary_store(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %q = call ptr @ordinary(%p)
    %before = load i64 %p
    store i64 7, %q
    %after = load i64 %p
    %sum = binary add i64 %before, %after
    return i64 %sum
}

# A pointer loaded after publication can reach the returned allocation.
function @published_alias(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %q = call ptr @fresh(%p)
    store ptr %q, @published
    store i64 3, %q
    %alias = call ptr @ordinary(%p)
    store i64 9, %alias
    %after = load i64 %q
    return i64 %after
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}

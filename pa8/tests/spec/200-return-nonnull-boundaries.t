declare function @fetch() -> ptr [return=nonnull]
function @invoke(%fn : ptr) -> ptr {
  block ^entry:
    %result = call ptr %fn() as () -> ptr [return=nonnull]
    return ptr %result
}

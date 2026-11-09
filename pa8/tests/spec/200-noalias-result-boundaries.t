declare function @allocate(%n : i64) -> ptr [return=nonnull, noalias_result=yes]
function @invoke(%fn : ptr) -> ptr {
  block ^entry:
    %result = call ptr %fn() as () -> ptr [noalias_result=yes]
    return ptr %result
}

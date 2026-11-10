function @scan(%selector : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^header
  block ^header:
    %i = phi i64 [^entry: 0, ^latch: %next]
    %more = cmp lt i64 %i, 4
    branch %more, ^body, ^exit
  block ^body:
    %key = binary add i64 %selector, %i
    switch %key, ^latch, 0:^exit, 1:^exit
  block ^latch:
    %next = binary add i64 %i, 1
    jump ^header
  block ^exit:
    %result = phi i64 [^header: %i, ^body: %i]
    return i64 %result
}
function @main() -> i32 [role=entry] {
  block ^entry:
    %one = call i64 @scan(-1)
    %none = call i64 @scan(2)
    %a = cmp eq i64 %one, 1
    %b = cmp eq i64 %none, 4
    %ok = binary and i64 %a, %b
    %status = select i32 %ok, 0, 1
    return i32 %status
}

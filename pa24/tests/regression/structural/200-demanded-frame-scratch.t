function @variadic_identity(%tag : i64) -> i64 [arity=variadic] {
  block ^entry:
    return i64 %tag
}
function @callee(%value : i64) -> i64 {
  block ^entry:
    return i64 %value
}
function @aliased_caller(%value : i64) -> i64 [keep_alias=yes] {
  block ^entry:
    %r = call i64 @callee(%value)
    return i64 %r
}
function @extended(%value : f80) -> i64 {
  block ^entry:
    %r = convert fptosi i64 f80 %value
    return i64 %r
}
function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call i64 @variadic_identity(17, 29)
    %b = call i64 @aliased_caller(31)
    %c = call i64 @extended(43.5L)
    %x = cmp ne i64 %a, 17
    %y = cmp ne i64 %b, 31
    %z = cmp ne i64 %c, 43
    %xy = binary or i64 %x, %y
    %bad = binary or i64 %xy, %z
    %exit = convert trunc i32 i64 %bad
    return i32 %exit
}

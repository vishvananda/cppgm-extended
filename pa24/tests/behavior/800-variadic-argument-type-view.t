function @sum(%tag : i64) -> i64 [arity=variadic] {
  slot $list : obj<24x8>
  block ^entry:
    %address = addr $list
    va_start %address
    %first = va_arg i64 %address
    %second = va_arg i64 %address
    %value = binary add i64 %first, %second
    return i64 %value
}
function @main() -> i64 [role=entry] {
  block ^entry:
    %wide = const i128 18446744073709551619
    %result = call i64 @sum(0, %wide, 7) [argument_type=1:i64, argument_type=2:i64]
    %correct = cmp eq i64 %result, 10
    %answer = select i64 %correct, 0, 1
    return i64 %answer
}

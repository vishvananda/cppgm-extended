function @store_boolean(%input : i8) -> i32 {
  slot $flag : i1
  block ^entry:
    store i1 %input, $flag
    %read = load u8 $flag
    %out = convert zext i32 u8 %read
    return i32 %out
}
function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call i32 @store_boolean(3)
    %b = call i32 @store_boolean(2)
    %c = call i32 @store_boolean(-1)
    %ab = binary add i32 %a, %b
    %sum = binary add i32 %ab, %c
    %bad = cmp ne i32 %sum, 2
    %out = convert zext i32 i1 %bad
    return i32 %out
}

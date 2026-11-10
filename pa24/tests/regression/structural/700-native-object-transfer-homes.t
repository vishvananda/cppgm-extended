global @result = {
  i64 0
  i64 0
}

function @make_pair() -> obj<16x8> [unwind=no] {
  slot $value : obj<16x8>
  block ^entry:
    %base = addr $value
    %next = index i8 %base, 8
    store i64 17, %base
    store i64 23, %next
    return obj<16x8> $value
}
function @disturb() -> obj<16x8> [unwind=no] {
  slot $value : obj<16x8>
  block ^entry:
    %base = addr $value
    %next = index i8 %base, 8
    store i64 99, %base
    store i64 100, %next
    return obj<16x8> $value
}
function @direct_pair() -> obj<16x8> [unwind=no] {
  slot $result : obj<16x8>
  block ^entry:
    %destination = addr $result
    %value = call obj<16x8> @make_pair()
    copyobj 16x8 %value, %destination
    return obj<16x8> $result
}
function @delayed_pair() -> obj<16x8> [unwind=no] {
  slot $result : obj<16x8>
  block ^entry:
    %destination = addr $result
    %value = call obj<16x8> @make_pair()
    %ignored = call obj<16x8> @disturb()
    copyobj 16x8 %value, %destination
    return obj<16x8> $result
}
function @make_small() -> obj<3x1> [unwind=no] {
  slot $value : obj<3x1>
  block ^entry:
    %base = addr $value
    %one = index i8 %base, 1
    %two = index i8 %base, 2
    store u8 42, %base
    store u8 43, %one
    store u8 44, %two
    return obj<3x1> $value
}
function @small_guard() -> i64 [unwind=no] {
  slot $buffer : obj<8x1>
  block ^entry:
    %base = addr $buffer
    %guard = index i8 %base, 3
    store u8 77, %guard
    %value = call obj<3x1> @make_small()
    copyobj 3x1 %value, %base
    %first = load u8 %base
    %two = index i8 %base, 2
    %last = load u8 %two
    %sentinel = load u8 %guard
    %bad0 = cmp ne u8 %first, 42
    %bad1 = cmp ne u8 %last, 44
    %bad2 = cmp ne u8 %sentinel, 77
    %bad01 = binary or i1 %bad0, %bad1
    %bad = binary or i1 %bad01, %bad2
    %exit = convert zext i64 i1 %bad
    return i64 %exit
}
function @clear(%p : ptr) -> void [unwind=no] {
  block ^entry:
    zeroinit 16x8 %p
    return void
}
function @param_first(%value : obj<16x8>) -> i64 [unwind=no] {
  slot $object : obj<16x8>
  block ^entry:
    %base = addr $object
    copyobj 16x8 %value, %base
    %next = index i8 %base, 8
    %a = load i64 %base
    %b = load i64 %next
    %sum = binary add i64 %a, %b
    return i64 %sum
}
function @param_touched(%value : obj<16x8>) -> i64 [unwind=no] {
  slot $object : obj<16x8>
  block ^entry:
    %base = addr $object
    zeroinit 16x8 %base
    copyobj 16x8 %value, %base
    %next = index i8 %base, 8
    %a = load i64 %base
    %b = load i64 %next
    %sum = binary add i64 %a, %b
    return i64 %sum
}
function @param_escaped(%value : obj<16x8>) -> i64 [unwind=no] {
  slot $object : obj<16x8>
  block ^entry:
    %base = addr $object
    call void @clear(%base)
    copyobj 16x8 %value, %base
    %next = index i8 %base, 8
    %a = load i64 %base
    %b = load i64 %next
    %sum = binary add i64 %a, %b
    return i64 %sum
}
function @param_twice(%value : obj<16x8>) -> i64 [unwind=no] {
  slot $first : obj<16x8>
  slot $second : obj<16x8>
  block ^entry:
    %first = addr $first
    %second = addr $second
    copyobj 16x8 %value, %first
    zeroinit 16x8 %first
    copyobj 16x8 %value, %second
    %next = index i8 %second, 8
    %a = load i64 %second
    %b = load i64 %next
    %sum = binary add i64 %a, %b
    return i64 %sum
}
function @main() -> i32 [role=entry, unwind=no] {
  block ^entry:
    %destination = addr @result
    %direct = call obj<16x8> @direct_pair()
    copyobj 16x8 %direct, %destination
    %a0 = load i64 %destination
    %next = index i8 %destination, 8
    %b0 = load i64 %next
    %bad0 = cmp ne i64 %a0, 17
    %bad1 = cmp ne i64 %b0, 23
    %any1 = binary or i1 %bad0, %bad1
    %delayed = call obj<16x8> @delayed_pair()
    copyobj 16x8 %delayed, %destination
    %a1 = load i64 %destination
    %b1 = load i64 %next
    %bad2 = cmp ne i64 %a1, 17
    %bad3 = cmp ne i64 %b1, 23
    %any2 = binary or i1 %any1, %bad2
    %any3 = binary or i1 %any2, %bad3
    %small = call i64 @small_guard()
    %bad4 = cmp ne i64 %small, 0
    %any4 = binary or i1 %any3, %bad4
    %pair = call obj<16x8> @make_pair()
    %first = call i64 @param_first(%pair)
    %bad5 = cmp ne i64 %first, 40
    %any5 = binary or i1 %any4, %bad5
    %touched = call i64 @param_touched(%pair)
    %bad6 = cmp ne i64 %touched, 40
    %any6 = binary or i1 %any5, %bad6
    %escaped = call i64 @param_escaped(%pair)
    %bad7 = cmp ne i64 %escaped, 40
    %any7 = binary or i1 %any6, %bad7
    %twice = call i64 @param_twice(%pair)
    %bad8 = cmp ne i64 %twice, 40
    %any8 = binary or i1 %any7, %bad8
    %exit = convert zext i32 i1 %any8
    return i32 %exit
}

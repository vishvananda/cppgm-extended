# Five parameters live across the call keep frame homes at baseline.
# The folded destination address follows its last pre-call register use;
# the call then replaces that register with the source pointer.
function @read_object(%source : ptr) -> obj<8x8> [unwind=no] {
  slot $result : obj<8x8>
  block ^entry:
    %result = addr $result
    %value = load i64 %source
    store i64 %value, %result
    return obj<8x8> $result
}
function @write_object(%destination : ptr, %source : ptr, %choice : i1, %a : i64, %b : i64, %c : i64) -> void [unwind=no] {
  slot $selected : i64
  block ^entry:
    %selected = addr $selected
    branch %choice, ^left, ^right
  block ^left:
    store i64 1, %selected
    jump ^join
  block ^right:
    store i64 2, %selected
    jump ^join
  block ^join:
    %tag = load i64 %selected
    store i64 %tag, %destination
    %field = index i8 %destination, 16
    %value = call obj<8x8> @read_object(%source)
    copyobj 8x8 %value, %field
    %again = load i64 %source
    store i64 %again, %destination
    %af = index i8 %destination, 24
    %bf = index i8 %destination, 32
    %cf = index i8 %destination, 40
    store i64 %a, %af
    store i64 %b, %bf
    store i64 %c, %cf
    return void
}
function @check(%choice : i1) -> i32 [unwind=no] {
  slot $destination : obj<48x8>
  slot $source : obj<24x8>
  block ^entry:
    %destination = addr $destination
    %source = addr $source
    %field = index i8 %destination, 16
    %guard = index i8 %source, 16
    store i64 0, %destination
    store i64 0, %field
    store i64 41, %source
    store i64 77, %guard
    call void @write_object(%destination, %source, %choice, 9, 10, 11)
    %value = load i64 %field
    %sentinel = load i64 %guard
    %first = load i64 %destination
    %bad0 = cmp ne i64 %value, 41
    %bad1 = cmp ne i64 %sentinel, 77
    %bad2 = cmp ne i64 %first, 41
    %bad01 = binary or i1 %bad0, %bad1
    %bad = binary or i1 %bad01, %bad2
    %exit = convert zext i32 i1 %bad
    return i32 %exit
}
function @main() -> i32 [role=entry, unwind=no] {
  block ^entry:
    %left = call i32 @check(1)
    %right = call i32 @check(0)
    %bad = binary or i32 %left, %right
    return i32 %bad
}

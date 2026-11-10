# Small private copies can feed scalar readers without a frame round trip.
global @data : obj<24x8> = { i64 4 i64 0 i64 9 }

function @flag(%p : ptr) -> i64 {
  slot $copy : i64
  block ^entry:
    %address = addr $copy
    copyobj 8x1 %p, %address
    %value = load i64 $copy
    %masked = binary and i64 %value, 4
    %result = cmp ne i64 %masked, 0
    return i64 %result
}

function @mix(%p : ptr, %x : i64) -> i64 {
  slot $copy : i64
  block ^entry:
    %field = index i64 %p, 2
    %address = addr $copy
    copyobj 8x1 %field, %address
    %value = load i64 $copy
    %result = binary xor i64 %x, %value
    return i64 %result
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %first = call i64 @flag(@data)
    %field = index i64 @data, 1
    %second = call i64 @flag(%field)
    %mixed = call i64 @mix(@data, 4660)
    %bad0 = cmp ne i64 %first, 1
    %bad1 = cmp ne i64 %second, 0
    %bad2 = cmp ne i64 %mixed, 4669
    %bad3 = binary or i64 %bad0, %bad1
    %bad = binary or i64 %bad2, %bad3
    %status = convert trunc i32 i64 %bad
    return i32 %status
}

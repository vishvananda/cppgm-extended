# The entry's tests of %form decide the loop bound's test on both edges
# into the loop, to different sides.
global @bytes : obj<8x8> = { i64 578437695752307201 }

function @decode_object_chunk(%data : ptr, %form : i32) -> i64 [no_inline=yes] {
  block ^entry:
    %not_three = cmp ne i32 %form, 3
    branch %not_three, ^not_three, ^loop

  block ^not_three:
    %not_four = cmp ne i32 %form, 4
    branch %not_four, ^reject, ^loop

  block ^reject:
    return i64 0

  block ^loop:
    %byte = phi u32 [^entry: 0, ^not_three: 0, ^body: %next]
    %value = phi i64 [^entry: 0, ^not_three: 0, ^body: %merged]
    %three = cmp eq i32 %form, 3
    branch %three, ^short, ^long

  block ^short:
    jump ^test

  block ^long:
    jump ^test

  block ^test:
    %bound = phi u32 [^short: 4, ^long: 8]
    %more = cmp ult u32 %byte, %bound
    branch %more, ^body, ^done

  block ^body:
    %at = index inbounds u8 %data, %byte
    %loaded = load u8 %at
    %wide = convert zext i64 u8 %loaded
    %bits = binary mul u32 %byte, 8
    %shift = convert zext i64 u32 %bits
    %placed = binary shl i64 %wide, %shift
    %merged = binary or i64 %value, %placed
    %next = binary add u32 %byte, 1
    jump ^loop

  block ^done:
    return i64 %value
}

function @main() -> i32 {
  block ^entry:
    %short = call i64 @decode_object_chunk(@bytes, 3)
    %long = call i64 @decode_object_chunk(@bytes, 4)
    %other = call i64 @decode_object_chunk(@bytes, 5)
    %short_ok = cmp eq i64 %short, 67305985
    %long_ok = cmp eq i64 %long, 578437695752307201
    %other_ok = cmp eq i64 %other, 0
    %short_bit = select i32 %short_ok, 0, 1
    %long_bit = select i32 %long_ok, 0, 2
    %other_bit = select i32 %other_ok, 0, 4
    %some = binary or i32 %short_bit, %long_bit
    %status = binary or i32 %some, %other_bit
    return i32 %status
}

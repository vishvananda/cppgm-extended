function @first(%p : ptr, %v : i64) -> void [no_inline=yes] {
  block ^entry:
    store i64 %v, %p
    return void
}
function @far(%p : ptr, %v : i64) -> void [no_inline=yes] {
  block ^entry:
    %last = index i8 %p, 24
    store i64 %v, %last
    return void
}
function @bounded_loop(%p : ptr [object_bytes=32], %n : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^loop
  block ^loop:
    %i = phi i64 [^entry: 0, ^body: %next]
    %sum = phi i64 [^entry: 0, ^body: %added]
    %more = cmp ult i64 %i, %n
    branch %more, ^body, ^done
  block ^body:
    %last = index i8 %p, 24
    %value = load i64 %last
    call void @first(%p, %i)
    %added = binary add i64 %sum, %value
    %next = binary add i64 %i, 1
    jump ^loop
  block ^done:
    return i64 %sum
}
function @exclusive_loop(%out : ptr [pass=indirect_result, alias=exclusive_result, object_bytes=32], %input : ptr [object_bytes=8], %n : i64) -> void [no_inline=yes] {
  block ^entry:
    zeroinit 32x8 %out
    jump ^loop
  block ^loop:
    %i = phi i64 [^entry: 0, ^body: %next]
    %sum = phi i64 [^entry: 0, ^body: %added]
    %more = cmp ult i64 %i, %n
    branch %more, ^body, ^done
  block ^body:
    %value = load i64 %input
    call void @first(%out, %i)
    %added = binary add i64 %sum, %value
    %next = binary add i64 %i, 1
    jump ^loop
  block ^done:
    store i64 %sum, %out
    return void
}
function @store_loop(%out : ptr [pass=indirect_result, alias=exclusive_result, object_bytes=32], %input : ptr [object_bytes=8], %n : i64) -> void [no_inline=yes] {
  block ^entry:
    zeroinit 32x8 %out
    jump ^loop
  block ^loop:
    %i = phi i64 [^entry: 0, ^body: %next]
    %sum = phi i64 [^entry: 0, ^body: %added]
    %more = cmp ult i64 %i, %n
    branch %more, ^body, ^done
  block ^body:
    %value = load i64 %input
    %last = index i8 %out, 24
    store i64 %i, %last
    %added = binary add i64 %sum, %value
    %next = binary add i64 %i, 1
    jump ^loop
  block ^done:
    store i64 %sum, %out
    return void
}
function @ordinary_loop(%p : ptr [object_bytes=8], %q : ptr, %n : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^loop
  block ^loop:
    %i = phi i64 [^entry: 0, ^body: %next]
    %sum = phi i64 [^entry: 0, ^body: %added]
    %more = cmp ult i64 %i, %n
    branch %more, ^body, ^done
  block ^body:
    %value = load i64 %p
    call void @first(%q, %i)
    %added = binary add i64 %sum, %value
    %next = binary add i64 %i, 1
    jump ^loop
  block ^done:
    return i64 %sum
}
function @outside_loop(%out : ptr [pass=indirect_result, alias=exclusive_result, object_bytes=8], %input : ptr [object_bytes=8], %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^loop
  block ^loop:
    %i = phi i64 [^entry: 0, ^body: %next]
    %sum = phi i64 [^entry: 0, ^body: %added]
    %more = cmp ult i64 %i, %n
    branch %more, ^body, ^done
  block ^body:
    %value = load i64 %input
    call void @far(%out, %i)
    %added = binary add i64 %sum, %value
    %next = binary add i64 %i, 1
    jump ^loop
  block ^done:
    store i64 %sum, %out
    return void
}
function @main() -> i32 [role=entry] {
  slot $storage : obj<64x8>
  slot $source : i64
  block ^entry:
    %out = addr $storage
    %input = addr $source
    %last = index i8 %out, 24
    store i64 11, %last
    %bounded = call i64 @bounded_loop(%out, 3)
    %bad1 = binary sub i64 %bounded, 33
    store i64 5, %input
    call void @exclusive_loop(%out, %input, 3)
    %exclusive = load i64 %out
    %bad2 = binary sub i64 %exclusive, 15
    %error2 = binary or i64 %bad1, %bad2
    call void @store_loop(%out, %input, 3)
    %stored = load i64 %out
    %bad3 = binary sub i64 %stored, 15
    %error3 = binary or i64 %error2, %bad3
    store i64 3, %out
    %ordinary = call i64 @ordinary_loop(%out, %out, 3)
    %bad4 = binary sub i64 %ordinary, 4
    %error4 = binary or i64 %error3, %bad4
    store i64 3, %last
    call void @outside_loop(%out, %last, 3)
    %outside = load i64 %out
    %bad5 = binary sub i64 %outside, 4
    %error5 = binary or i64 %error4, %bad5
    %failed = cmp ne i64 %error5, 0
    %status = convert zext i32 i1 %failed
    return i32 %status
}

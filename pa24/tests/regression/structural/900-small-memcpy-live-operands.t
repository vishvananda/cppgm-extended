function @memory_copy(%destination : ptr, %source : ptr, %count : i64) -> ptr
    [unwind=no, object=cppgm_builtin_memcpy] {
  block ^entry:
    jump ^test
  block ^test:
    %index = phi i64 [^entry: 0, ^body: %next]
    %done = cmp eq i64 %index, %count
    branch %done, ^return, ^body
  block ^body:
    %from = index i8 %source, %index
    %byte = load u8 %from
    %to = index i8 %destination, %index
    store u8 %byte, %to
    %next = binary add i64 %index, 1
    jump ^test
  block ^return:
    return ptr %destination
}

function @copy_keep(%destination : ptr, %source : ptr, %value : i64) -> i64 [unwind=no] {
  block ^entry:
    %ignored = call ptr @memory_copy(%destination, %source, 13)
    %copied = load i64 %destination
    %result = binary add i64 %copied, %value
    return i64 %result
}

function @read_fixed(%source : ptr, %value : i64) -> i64 [unwind=no] {
  slot $word : i64
  block ^entry:
    %destination = addr $word
    %ignored = call ptr @memory_copy(%destination, %source, 8)
    %copied = load i64 $word
    %result = binary add i64 %copied, %value
    return i64 %result
}

function @copy_pressure(%destination : ptr, %source : ptr, %a : i64, %b : i64, %c : i64, %d : i64, %e : i64, %f : i64) -> i64 [unwind=no] {
  block ^entry:
    %ignored = call ptr @memory_copy(%destination, %source, 32)
    %copied = load i64 %destination
    %ab = binary add i64 %a, %b
    %cd = binary add i64 %c, %d
    %ef = binary add i64 %e, %f
    %abcd = binary add i64 %ab, %cd
    %all = binary add i64 %abcd, %ef
    %result = binary add i64 %all, %copied
    return i64 %result
}

function @zero_copy(%destination : ptr, %source : ptr, %value : i64) -> i64 [unwind=no] {
  block ^entry:
    %ignored = call ptr @memory_copy(%destination, %source, 0)
    return i64 %value
}

function @main() -> i32 [role=entry, unwind=no] {
  slot $source : obj<48x1>
  slot $destination : obj<48x1>
  block ^entry:
    zeroinit 48x1 $source
    zeroinit 48x1 $destination
    %source_start = addr $source
    %destination_start = addr $destination
    %source = index i8 %source_start, 1
    %destination = index i8 %destination_start, 3
    store i64 42, %source
    %last_source = index i8 %source, 12
    store u8 77, %last_source
    %last32_source = index i8 %source, 31
    store u8 88, %last32_source
    %sentinel = index i8 %destination, 32
    store u8 99, %sentinel
    %kept = call i64 @copy_keep(%destination, %source, 19)
    %bad_kept = cmp ne i64 %kept, 61
    %read = call i64 @read_fixed(%destination, 31)
    %bad_read = cmp ne i64 %read, 73
    %pressure = call i64 @copy_pressure(%destination, %source, 1, 2, 3, 4, 5, 6)
    %bad_pressure = cmp ne i64 %pressure, 63
    %zero = call i64 @zero_copy(%destination, %source, 91)
    %bad_zero = cmp ne i64 %zero, 91
    %last_destination = index i8 %destination, 12
    %last = load u8 %last_destination
    %bad_last = cmp ne u8 %last, 77
    %last32_destination = index i8 %destination, 31
    %last32 = load u8 %last32_destination
    %bad_last32 = cmp ne u8 %last32, 88
    %after = load u8 %sentinel
    %bad_after = cmp ne u8 %after, 99
    %bad0 = binary or i1 %bad_kept, %bad_read
    %bad1 = binary or i1 %bad_pressure, %bad_zero
    %bad2 = binary or i1 %bad_last, %bad_last32
    %bad3 = binary or i1 %bad0, %bad1
    %bad4 = binary or i1 %bad2, %bad_after
    %bad = binary or i1 %bad3, %bad4
    %exit = convert zext i32 i1 %bad
    return i32 %exit
}

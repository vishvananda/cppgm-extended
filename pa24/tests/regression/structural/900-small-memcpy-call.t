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

function @copy_used(%destination : ptr, %source : ptr) -> ptr [unwind=no] {
  block ^entry:
    %length = convert sext i64 i32 8
    %result = call ptr @memory_copy(%destination, %source, %length)
    return ptr %result
}

function @copy_unused(%destination : ptr, %source : ptr) -> void [unwind=no] {
  block ^entry:
    %ignored = call ptr @memory_copy(%destination, %source, 13)
    return void
}

function @copy_dynamic(%destination : ptr, %source : ptr, %count : i64) -> ptr [unwind=no] {
  block ^entry:
    %result = call ptr @memory_copy(%destination, %source, %count)
    return ptr %result
}

function @ordinary_copy(%destination : ptr, %source : ptr, %count : i64) -> ptr [unwind=no] {
  block ^entry:
    return ptr %destination
}

function @ordinary_call(%destination : ptr, %source : ptr) -> ptr [unwind=no] {
  block ^entry:
    %result = call ptr @ordinary_copy(%destination, %source, 8)
    return ptr %result
}

function @main() -> i32 [role=entry, unwind=no] {
  slot $source : obj<40x1>
  slot $destination : obj<40x1>
  block ^entry:
    zeroinit 40x1 $source
    zeroinit 40x1 $destination
    %source_start = addr $source
    %destination_start = addr $destination
    %source = index i8 %source_start, 1
    %destination = index i8 %destination_start, 3
    store i64 42, %source
    %last_source = index i8 %source, 12
    store u8 77, %last_source
    %sentinel = index i8 %destination, 13
    store u8 99, %sentinel
    %result = call ptr @copy_used(%destination, %source)
    %bad_result = cmp ne ptr %result, %destination
    call void @copy_unused(%destination, %source)
    %first = load i64 %destination
    %bad_first = cmp ne i64 %first, 42
    %last_destination = index i8 %destination, 12
    %last = load u8 %last_destination
    %bad_last = cmp ne u8 %last, 77
    %after = load u8 %sentinel
    %bad_after = cmp ne u8 %after, 99
    %bad0 = binary or i1 %bad_result, %bad_first
    %bad1 = binary or i1 %bad_last, %bad_after
    %bad = binary or i1 %bad0, %bad1
    %exit = convert zext i32 i1 %bad
    return i32 %exit
}

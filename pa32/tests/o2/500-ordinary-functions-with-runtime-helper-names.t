global @calls : i64 = 0
global @copy_target = { i8 0 i8 0 i8 0 }
global @source [storage=readonly] = { i8 97 i8 98 i8 0 }
global @destination = { i8 9 i8 0 i8 0 }

function @cppgm_builtin_strlen(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %counter = addr @calls
    %old = load i64 %counter
    %next = binary add i64 %old, 1
    store i64 %next, %counter
    return i64 117
}

function @cppgm_builtin_memset(%p : ptr, %v : i32, %n : i64) -> ptr [no_inline=yes] {
  block ^entry:
    %counter = addr @calls
    %old = load i64 %counter
    %next = binary add i64 %old, 1
    store i64 %next, %counter
    return ptr %p
}

function @cppgm_builtin_memcpy(%p : ptr, %s : ptr, %n : i64) -> ptr [no_inline=yes] {
  block ^entry:
    %counter = addr @calls
    %old = load i64 %counter
    %next = binary add i64 %old, 1
    store i64 %next, %counter
    return ptr %p
}

function @cppgm_builtin_memcmp(%p : ptr, %s : ptr, %n : i64) -> i32 [no_inline=yes] {
  block ^entry:
    %counter = addr @calls
    %old = load i64 %counter
    %next = binary add i64 %old, 1
    store i64 %next, %counter
    return i32 9
}

function @__cppgm_builtin_memset(%p : ptr, %v : i32, %n : i64) -> ptr [no_inline=yes] {
  block ^entry:
    return ptr %p
}
function @fill_loop(%p : ptr, %end : ptr) -> void [no_inline=yes] {
  block ^entry:
    jump ^test
  block ^test:
    %q = phi ptr [^entry: %p, ^more: %next]
    %done = cmp eq ptr %q, %end
    branch %done, ^return, ^more
  block ^more:
    store i8 7, %q
    %next = index i8 %q, 1
    jump ^test
  block ^return:
    return void
}

function @__cppgm_builtin_memmove(%p : ptr, %s : ptr, %n : i64) -> ptr [no_inline=yes] {
  block ^entry:
    return ptr %p
}
function @copy_loop(%to : ptr, %from : ptr, %end : ptr) -> void [no_inline=yes] {
  block ^entry:
    jump ^test
  block ^test:
    %p = phi ptr [^entry: %from, ^more: %next_p]
    %q = phi ptr [^entry: %to, ^more: %next_q]
    %done = cmp eq ptr %p, %end
    branch %done, ^return, ^more
  block ^more:
    %v = load i8 %p
    store i8 %v, %q
    %next_p = index i8 %p, 1
    %next_q = index i8 %q, 1
    jump ^test
  block ^return:
    return void
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %from = addr @source
    %to = addr @destination
    %end = index i8 %to, 3
    call void @fill_loop(%to, %end)
    %copy_to = addr @copy_target
    %copy_end = index i8 %from, 3
    call void @copy_loop(%copy_to, %from, %copy_end)
    %first = load i8 %copy_to
    %bad_copy = cmp ne i8 %first, 97
    branch %bad_copy, ^fail, ^length
  block ^length:
    %length = call i64 @cppgm_builtin_strlen(%from)
    %bad_length = cmp ne i64 %length, 117
    branch %bad_length, ^fail, ^fill
  block ^fill:
    %filled = call ptr @cppgm_builtin_memset(%to, 0, 1)
    %copied = call ptr @cppgm_builtin_memcpy(%to, %from, 1)
    %byte = load i8 %to
    %bad_byte = cmp ne i8 %byte, 7
    branch %bad_byte, ^fail, ^compare
  block ^compare:
    %compared = call i32 @cppgm_builtin_memcmp(%from, %from, 2)
    %is_zero = cmp eq i32 %compared, 0
    branch %is_zero, ^fail, ^count
  block ^count:
    %counter = addr @calls
    %total = load i64 %counter
    %bad_count = cmp ne i64 %total, 4
    branch %bad_count, ^fail, ^done
  block ^fail:
    return i32 1
  block ^done:
    return i32 0
}

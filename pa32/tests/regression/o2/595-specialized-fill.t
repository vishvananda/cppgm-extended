# Every call passes 0: the specialized loop stores 0, a fill. memset's
# declaration is newer than the call graph the specialization reads.

global @buffer : obj<4096x4> = { zero 4096 }

function @scale(%first : ptr, %last : ptr, %k : i32) -> void [binding=internal, no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %p = phi ptr [^entry: %first, ^body: %next]
    %more = cmp ne ptr %p, %last
    branch %more, ^body, ^done
  block ^body:
    %old = load i32 %p
    %value = binary mul i32 %old, %k
    store i32 %value, %p
    %next = index i8 %p, 4
    jump ^head
  block ^done:
    return void
}

function @main(%n : i64) -> i32 [role=entry] {
  block ^entry:
    %first = addr @buffer
    %last = index i8 %first, %n
    call void @scale(%first, %last, 0)
    return i32 0
}

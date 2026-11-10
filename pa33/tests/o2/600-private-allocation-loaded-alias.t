declare function @malloc(%bytes : i64) -> ptr [linkage=c, unwind=no]
declare function @free(%p : ptr) -> void [linkage=c, unwind=no]
declare function @capture(%p : ptr) -> void [linkage=c]
global @published : ptr = 0

# Loading a pointer from an older object does not expose this private buffer.
function @private_scan(%holder : ptr) -> i32 [no_inline=yes] {
  block ^entry:
    %buffer = call ptr @malloc(16)
    jump ^loop
  block ^loop:
    %i = phi i32 [^entry: 0, ^loop: %next]
    %sum = phi i32 [^entry: 0, ^loop: %total]
    %source = load ptr %holder
    %value = load i32 %source
    %at = index i32 %buffer, %i
    store i32 %i, %at
    %total = binary add i32 %sum, %value
    %next = binary add i32 %i, 1
    %more = cmp lt i32 %next, 4
    branch %more, ^loop, ^done
  block ^done:
    %last = index i32 %buffer, 3
    %observed = load volatile i32 %last
    call void @free(%buffer)
    %out = binary add i32 %total, %observed
    return i32 %out
}

# A published address can be loaded again through the caller's holder.
function @published_scan(%holder : ptr, %n : i32) -> i32 [no_inline=yes] {
  block ^entry:
    %buffer = call ptr @malloc(16)
    store ptr %buffer, @published
    store i32 7, %buffer
    jump ^loop
  block ^loop:
    %i = phi i32 [^entry: 0, ^loop: %next]
    %sum = phi i32 [^entry: 0, ^loop: %total]
    %source = load ptr %holder
    %value = load i32 %source
    store i32 %i, %buffer
    %total = binary add i32 %sum, %value
    %next = binary add i32 %i, 1
    %more = cmp lt i32 %next, %n
    branch %more, ^loop, ^done
  block ^done:
    call void @free(%buffer)
    return i32 %total
}

# An opaque callee may publish its argument even if this body never does.
function @captured_scan(%holder : ptr, %n : i32) -> i32 [no_inline=yes] {
  block ^entry:
    %buffer = call ptr @malloc(16)
    call void @capture(%buffer)
    store i32 7, %buffer
    jump ^loop
  block ^loop:
    %i = phi i32 [^entry: 0, ^loop: %next]
    %sum = phi i32 [^entry: 0, ^loop: %total]
    %source = load ptr %holder
    %value = load i32 %source
    store i32 %i, %buffer
    %total = binary add i32 %sum, %value
    %next = binary add i32 %i, 1
    %more = cmp lt i32 %next, %n
    branch %more, ^loop, ^done
  block ^done:
    call void @free(%buffer)
    return i32 %total
}
function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}

# Return the matching bucket or the first tombstone without extra result copies.
function @probe(%array : ptr, %key : ptr, %mask : u32) -> ptr [no_inline=yes] {
  block ^entry:
    jump ^loop
  block ^loop:
    %bucket = phi u32 [^entry: 0, ^step: %masked]
    %amount = phi u32 [^entry: 1, ^step: %next]
    %tomb = phi ptr [^entry: 0, ^step: %saved]
    %address = index ptr %array, %bucket
    %value = load ptr %address
    %empty = cmp eq ptr %value, -1
    branch %empty, ^empty_bucket, ^occupied
  block ^empty_bucket:
    branch %tomb, ^old_tomb, ^fresh
  block ^fresh:
    %result_address = index ptr %array, %bucket
    jump ^end
  block ^old_tomb:
    jump ^end
  block ^occupied:
    %match = cmp eq ptr %value, %key
    branch %match, ^found, ^check_tomb
  block ^found:
    %found_address = index ptr %array, %bucket
    jump ^end
  block ^check_tomb:
    %dead = cmp eq ptr %value, -2
    branch %dead, ^check_null, ^keep
  block ^check_null:
    %no_tomb = cmp eq ptr %tomb, 0
    branch %no_tomb, ^remember, ^keep
  block ^remember:
    %new_tomb = index ptr %array, %bucket
    jump ^step
  block ^keep:
    jump ^step
  block ^step:
    %saved = phi ptr [^remember: %new_tomb, ^keep: %tomb]
    %next = binary add u32 %amount, 1
    %sum = binary add u32 %bucket, %amount
    %masked = binary and u32 %sum, %mask
    jump ^loop
  block ^end:
    %out = phi ptr [^fresh: %result_address, ^old_tomb: %tomb, ^found: %found_address]
    return ptr %out
}
function @main() -> i64 [role=entry] {
 slot $data : obj<64x8>
 block ^entry:
  %p = addr $data
  store ptr -2, %p
  %one = index ptr %p, 1
  store ptr 3, %one
  %three = index ptr %p, 3
  store ptr -1, %three
  %a = call ptr @probe(%p, 3, 7)
  %b = call ptr @probe(%p, 4, 7)
  %bad_a = cmp ne ptr %a, %one
  %bad_b = cmp ne ptr %b, %p
  %bad = binary or i64 %bad_a, %bad_b
  return i64 %bad
}

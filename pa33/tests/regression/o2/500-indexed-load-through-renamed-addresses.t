# A bit chooses between two fields of one object, and each arm renames its
# field's address before the load (a smart pointer's accessor does this): one
# load through the address the bit selects.
function @child(%node : ptr, %bits : i64) -> ptr [binding=strong, no_inline=yes] {
  block ^entry:
    %bit = binary and i64 %bits, 8
    %set = cmp ne i64 %bit, 0
    branch %set, ^right, ^left
  block ^right:
    %right_at = index i8 %node, 24
    %right_this = copy nonnull ptr %right_at
    %right_value = load typed ptr %right_this
    jump ^join
  block ^left:
    %left_at = index i8 %node, 8
    %left_this = copy nonnull ptr %left_at
    %left_value = load typed ptr %left_this
    jump ^join
  block ^join:
    %value = phi ptr [^right: %right_value, ^left: %left_value]
    return ptr %value
}

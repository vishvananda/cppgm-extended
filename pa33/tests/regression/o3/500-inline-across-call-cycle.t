# At O3, a callee on a call cycle that neither loops nor calls itself goes
# inline as any other callee does. @walk, @route, @pick and @gate call each
# other in a ring: @walk takes @route with @pick, and so does @outside,
# which is not on the ring. A callee on the ring that loops keeps its call
# (@walk, from @gate), and so does one that calls itself (@ping, from @pong
# and from @outside).
function @pick(%node : ptr, %depth : i64) -> i64 [binding=strong] {
  block ^entry:
    branch %depth, ^down, ^done

  block ^down:
    %link = index i8 %node, 16
    %child = load ptr %link
    %less = binary sub i64 %depth, 1
    %r = call i64 @gate(%child, %less)
    return i64 %r

  block ^done:
    return i64 0
}

function @route(%node : ptr, %depth : i64) -> i64 [binding=strong] {
  block ^entry:
    %kind = load i64 %node
    %at = index i8 %node, 8
    %value = load i64 %at
    branch %kind, ^inner, ^leaf

  block ^inner:
    %below = call i64 @pick(%node, %depth)
    %sum = binary add i64 %below, %value
    return i64 %sum

  block ^leaf:
    return i64 %value
}

function @walk(%first : ptr, %depth : i64) -> i64 [binding=strong] {
  block ^entry:
    jump ^head

  block ^head:
    %node = phi ptr [^entry: %first, ^body: %next]
    %total = phi i64 [^entry: 0, ^body: %more]
    %live = cmp ne ptr %node, 0
    branch %live, ^body, ^done

  block ^body:
    %one = call i64 @route(%node, %depth)
    %more = binary add i64 %total, %one
    %link = index i8 %node, 24
    %next = load ptr %link
    jump ^head

  block ^done:
    return i64 %total
}

function @gate(%node : ptr, %depth : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %r = call i64 @walk(%node, %depth)
    return i64 %r
}

function @pong(%n : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %r = call i64 @ping(%n)
    %s = binary add i64 %r, 1
    return i64 %s
}

function @ping(%n : i64) -> i64 [binding=strong] {
  block ^entry:
    branch %n, ^more, ^done

  block ^more:
    %less = binary sub i64 %n, 1
    %a = call i64 @pong(%less)
    %half = binary ushr i64 %less, 1
    %b = call i64 @ping(%half)
    %r = binary add i64 %a, %b
    return i64 %r

  block ^done:
    return i64 1
}

function @outside(%node : ptr, %depth : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %r = call i64 @route(%node, %depth)
    %p = call i64 @ping(%depth)
    %s = binary add i64 %r, %p
    return i64 %s
}

function @main() -> i32 [role=entry] {
  slot $leaf_node : obj<32x8>
  slot $root_node : obj<32x8>

  block ^entry:
    %leaf = addr $leaf_node
    %root = addr $root_node
    store i64 0, %leaf
    %leaf_value = index i8 %leaf, 8
    store i64 5, %leaf_value
    %leaf_child = index i8 %leaf, 16
    store ptr 0, %leaf_child
    %leaf_next = index i8 %leaf, 24
    store ptr 0, %leaf_next
    store i64 1, %root
    %root_value = index i8 %root, 8
    store i64 7, %root_value
    %root_child = index i8 %root, 16
    store ptr %leaf, %root_child
    %root_next = index i8 %root, 24
    store ptr %leaf, %root_next
    %r = call i64 @outside(%root, 3)
    %t = convert trunc i32 i64 %r
    return i32 %t
}

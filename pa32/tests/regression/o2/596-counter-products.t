# A product of a counter and a literal becomes a counter of its own, here and
# in the loop's other reads of the same product.

function @rows(%a : ptr, %n : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 1, ^body: %next]
    %s = phi i64 [^entry: 0, ^body: %sum]
    %m = binary mul i64 %i, 164
    %p = index i8 %a, %m
    %v = load i64 %p
    %m2 = binary mul i64 %i, 164
    %q = index i8 %p, %m2
    %w = load i64 %q
    %x = binary add i64 %s, %v
    %sum = binary add i64 %x, %w
    %next = binary add i64 %i, 2
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return i64 %sum
}

# A 32-bit counter that does not wrap widens with its steps, down as well.
function @down(%a : ptr, %n : i32) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i32 [^entry: %n, ^body: %next]
    %s = phi i64 [^entry: 0, ^body: %sum]
    %w = convert sext i64 i32 %i
    %m = binary mul i64 %w, 164
    %p = index i8 %a, %m
    %v = load i64 %p
    %sum = binary add i64 %s, %v
    %next = binary sub nowrap i32 %i, 1
    %c = cmp gt i32 %next, 0
    branch %c, ^body, ^done
  block ^done:
    return i64 %sum
}

# Refused: the 32-bit counter may wrap before it widens.
function @wraps(%a : ptr, %n : i32) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i32 [^entry: 0, ^body: %next]
    %s = phi i64 [^entry: 0, ^body: %sum]
    %w = convert sext i64 i32 %i
    %m = binary mul i64 %w, 164
    %p = index i8 %a, %m
    %v = load i64 %p
    %sum = binary add i64 %s, %v
    %next = binary add i32 %i, 1
    %c = cmp ne i32 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return i64 %sum
}

# Refused: `lea` gives these.
function @cheap(%a : ptr, %n : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi i64 [^entry: 0, ^body: %sum]
    %m = binary mul i64 %i, 9
    %t = binary mul i64 %i, 8
    %x = binary add i64 %s, %m
    %sum = binary add i64 %x, %t
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return i64 %sum
}

declare function @use(%x : i64) -> void [linkage=c, binding=strong]

# Refused: the new counter would cross the call.
function @calls(%n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %m = binary mul i64 %i, 164
    call void @use(%m)
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}

# A stride the loop does not change steps the same way, and its products need a
# preheader behind the guard.
function @stride(%a : ptr, %s : i32, %n : i32) -> i64 [no_inline=yes] {
  block ^entry:
    %g = cmp gt i32 %n, 0
    branch %g, ^body, ^done
  block ^body:
    %i = phi i32 [^entry: 0, ^body: %next]
    %t = phi i64 [^entry: 0, ^body: %sum]
    %m = binary mul nowrap i32 %s, %i
    %p = index i64 %a, %m
    %v = load i64 %p
    %sum = binary add i64 %t, %v
    %next = binary add nowrap i32 %i, 1
    %c = cmp lt i32 %next, %n
    branch %c, ^body, ^done
  block ^done:
    %r = phi i64 [^entry: 0, ^body: %sum]
    return i64 %r
}

# From a start of its own: the first value multiplies, and the step does when the
# counter does not step by one.
function @offset(%a : ptr, %s : i64, %from : i64, %n : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: %from, ^body: %next]
    %t = phi i64 [^entry: 0, ^body: %sum]
    %m = binary mul i64 %i, %s
    %p = index i8 %a, %m
    %v = load i64 %p
    %sum = binary add i64 %t, %v
    %next = binary add i64 %i, 3
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return i64 %sum
}

# A value the loop computes is no stride.
function @changing(%a : ptr, %n : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %t = phi i64 [^entry: 1, ^body: %sum]
    %m = binary mul i64 %i, %t
    %p = index i8 %a, %m
    %v = load i64 %p
    %sum = binary add i64 %t, %v
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return i64 %sum
}

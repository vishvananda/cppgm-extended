# A floating sum keeps its passes' order: it stays scalar and adds the lanes one
# at a time, when a costlier operation runs in the lanes.

function @dot(%x : ptr, %y : ptr, %n : i64) -> f64 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi f64 [^entry: 0.5, ^body: %sum]
    %p = index f64 %x, %i
    %v = load typed f64 %p
    %q = index f64 %y, %i
    %w = load typed f64 %q
    %m = binary mul f64 %v, %w
    %sum = binary add f64 %s, %m
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return f64 %sum
}

# Four lanes of a subtraction.
function @quotients(%x : ptr, %y : ptr, %n : i64) -> f32 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi f32 [^entry: 1, ^body: %sum]
    %p = index f32 %x, %i
    %v = load typed f32 %p
    %q = index f32 %y, %i
    %w = load typed f32 %q
    %m = binary div f32 %v, %w
    %sum = binary sub f32 %s, %m
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return f32 %sum
}

# Walking down, the top lane holds the first pass.
function @down(%x : ptr, %n : i64) -> f64 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: %n, ^body: %next]
    %s = phi f64 [^entry: 0, ^body: %sum]
    %p = index f64 %x, %i
    %v = load typed f64 %p
    %m = binary mul f64 %v, 3
    %sum = binary add f64 %m, %s
    %next = binary sub i64 %i, 1
    %c = cmp gt i64 %next, 0
    branch %c, ^body, ^done
  block ^done:
    return f64 %sum
}

# A product, of a value built from the counter: halved 32-bit lanes.
function @halves(%n : i32) -> f64 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i32 [^entry: 1, ^body: %next]
    %s = phi f64 [^entry: 1, ^body: %sum]
    %t = binary mul nowrap i32 %i, %i
    %h = binary div i32 %t, 4
    %f = convert sitofp f64 i32 %h
    %r = binary div f64 1, %f
    %sum = binary mul f64 %s, %r
    %next = binary add nowrap i32 %i, 1
    %c = cmp lt i32 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return f64 %sum
}

# Nothing costlier than the sum runs in the lanes: the loop stays scalar.
function @plain(%x : ptr, %n : i64) -> f64 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi f64 [^entry: 0, ^body: %sum]
    %p = index f64 %x, %i
    %v = load typed f64 %p
    %sum = binary add f64 %s, %v
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return f64 %sum
}

# A sum another operation reads each pass needs its running value in each lane.
function @running(%x : ptr, %n : i64) -> f64 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi f64 [^entry: 0, ^body: %sum]
    %p = index f64 %x, %i
    %v = load typed f64 %p
    %m = binary mul f64 %v, %s
    %sum = binary add f64 %s, %m
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return f64 %sum
}

# A division that is no halving stays in one lane.
function @thirds(%x : ptr, %n : i64) -> i32 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi i32 [^entry: 0, ^body: %sum]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %h = binary div i32 %v, 3
    %sum = binary add i32 %s, %h
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return i32 %sum
}

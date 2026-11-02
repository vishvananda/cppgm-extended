# A nest that folds a sum over a stride and stores it in the outer loop swaps its
# loops: a loop of first values, then each sum's cell takes one term of each row
# in the inner loop, and that loop runs in lanes.

global @out : obj<8192x8> = { zero 8192 }
global @left : obj<8192x8> = { zero 8192 }
global @right : obj<8192x8> = { zero 8192 }
global @counts : obj<4096x4> = { zero 4096 }
global @ints : obj<4096x4> = { zero 4096 }

function @product(%i : i64) -> void [no_inline=yes] {
  block ^entry:
    %r = addr @out
    %a = addr @left
    %b = addr @right
    %row = binary mul i64 %i, 256
    %ri = index i8 %r, %row
    %ai = index i8 %a, %row
    jump ^columns
  block ^columns:
    %j = phi i64 [^entry: 0, ^next: %j1]
    %cell = index f64 %ri, %j
    store typed f64 0, %cell
    jump ^terms
  block ^terms:
    %k = phi i64 [^columns: 0, ^terms: %k1]
    %s = phi f64 [^columns: 0, ^terms: %sum]
    %p = index f64 %ai, %k
    %v = load typed f64 %p
    %scaled = binary mul i64 %k, 256
    %rowk = index i8 %b, %scaled
    %q = index f64 %rowk, %j
    %w = load typed f64 %q
    %m = binary mul f64 %v, %w
    %sum = binary add f64 %s, %m
    %k1 = binary add i64 %k, 1
    %c = cmp lt i64 %k1, 32
    branch %c, ^terms, ^next
  block ^next:
    store typed f64 %sum, %cell
    %j1 = binary add i64 %j, 1
    %d = cmp lt i64 %j1, 32
    branch %d, ^columns, ^done
  block ^done:
    return void
}

# No first store: the swapped loops need one, of the sum's start.
function @started() -> void [no_inline=yes] {
  block ^entry:
    %r = addr @counts
    %b = addr @ints
    jump ^columns
  block ^columns:
    %j = phi i64 [^entry: 0, ^next: %j1]
    %cell = index i32 %r, %j
    jump ^terms
  block ^terms:
    %k = phi i64 [^columns: 0, ^terms: %k1]
    %s = phi i32 [^columns: 5, ^terms: %sum]
    %scaled = binary mul i64 %k, 128
    %rowk = index i8 %b, %scaled
    %q = index i32 %rowk, %j
    %w = load typed i32 %q
    %sum = binary add i32 %s, %w
    %k1 = binary add i64 %k, 1
    %c = cmp lt i64 %k1, 32
    branch %c, ^terms, ^next
  block ^next:
    store typed i32 %sum, %cell
    %j1 = binary add i64 %j, 1
    %d = cmp lt i64 %j1, 32
    branch %d, ^columns, ^done
  block ^done:
    return void
}

# The inner loop reads what the outer loop stores: the loops stay.
function @reads_cell() -> void [no_inline=yes] {
  block ^entry:
    %r = addr @out
    jump ^columns
  block ^columns:
    %j = phi i64 [^entry: 0, ^next: %j1]
    %cell = index f64 %r, %j
    jump ^terms
  block ^terms:
    %k = phi i64 [^columns: 0, ^terms: %k1]
    %s = phi f64 [^columns: 0, ^terms: %sum]
    %scaled = binary mul i64 %k, 256
    %rowk = index i8 %r, %scaled
    %q = index f64 %rowk, %j
    %w = load typed f64 %q
    %sum = binary add f64 %s, %w
    %k1 = binary add i64 %k, 1
    %c = cmp lt i64 %k1, 32
    branch %c, ^terms, ^next
  block ^next:
    store typed f64 %sum, %cell
    %j1 = binary add i64 %j, 1
    %d = cmp lt i64 %j1, 32
    branch %d, ^columns, ^done
  block ^done:
    return void
}

# The inner count changes with the outer pass: the loops stay.
function @triangle() -> void [no_inline=yes] {
  block ^entry:
    %r = addr @out
    %b = addr @right
    jump ^columns
  block ^columns:
    %j = phi i64 [^entry: 0, ^next: %j1]
    %cell = index f64 %r, %j
    jump ^terms
  block ^terms:
    %k = phi i64 [^columns: 0, ^terms: %k1]
    %s = phi f64 [^columns: 0, ^terms: %sum]
    %scaled = binary mul i64 %k, 256
    %rowk = index i8 %b, %scaled
    %q = index f64 %rowk, %j
    %w = load typed f64 %q
    %sum = binary add f64 %s, %w
    %k1 = binary add i64 %k, 1
    %c = cmp le i64 %k1, %j
    branch %c, ^terms, ^next
  block ^next:
    store typed f64 %sum, %cell
    %j1 = binary add i64 %j, 1
    %d = cmp lt i64 %j1, 32
    branch %d, ^columns, ^done
  block ^done:
    return void
}

# The inner loop already walks consecutive elements: the loops stay.
function @consecutive() -> void [no_inline=yes] {
  block ^entry:
    %r = addr @out
    %b = addr @right
    jump ^columns
  block ^columns:
    %j = phi i64 [^entry: 0, ^next: %j1]
    %cell = index f64 %r, %j
    jump ^terms
  block ^terms:
    %k = phi i64 [^columns: 0, ^terms: %k1]
    %s = phi f64 [^columns: 0, ^terms: %sum]
    %q = index f64 %b, %k
    %w = load typed f64 %q
    %m = binary mul f64 %w, %w
    %sum = binary add f64 %s, %m
    %k1 = binary add i64 %k, 1
    %c = cmp lt i64 %k1, 32
    branch %c, ^terms, ^next
  block ^next:
    store typed f64 %sum, %cell
    %j1 = binary add i64 %j, 1
    %d = cmp lt i64 %j1, 32
    branch %d, ^columns, ^done
  block ^done:
    return void
}

# The inner counter starts from the outer one: the loops stay.
function @from_outer() -> void [no_inline=yes] {
  block ^entry:
    %r = addr @out
    %b = addr @right
    jump ^columns
  block ^columns:
    %j = phi i64 [^entry: 0, ^next: %j1]
    %cell = index f64 %r, %j
    jump ^terms
  block ^terms:
    %k = phi i64 [^columns: %j, ^terms: %k1]
    %s = phi f64 [^columns: 0, ^terms: %sum]
    %scaled = binary mul i64 %k, 256
    %rowk = index i8 %b, %scaled
    %q = index f64 %rowk, %j
    %w = load typed f64 %q
    %sum = binary add f64 %s, %w
    %k1 = binary add i64 %k, 1
    %c = cmp lt i64 %k1, 32
    branch %c, ^terms, ^next
  block ^next:
    store typed f64 %sum, %cell
    %j1 = binary add i64 %j, 1
    %d = cmp lt i64 %j1, 32
    branch %d, ^columns, ^done
  block ^done:
    return void
}

# The sum starts from the outer counter: the loops stay.
function @varied_start() -> void [no_inline=yes] {
  block ^entry:
    %r = addr @out
    %b = addr @right
    jump ^columns
  block ^columns:
    %j = phi i64 [^entry: 0, ^next: %j1]
    %cell = index f64 %r, %j
    %f = convert sitofp f64 i64 %j
    jump ^terms
  block ^terms:
    %k = phi i64 [^columns: 0, ^terms: %k1]
    %s = phi f64 [^columns: %f, ^terms: %sum]
    %scaled = binary mul i64 %k, 256
    %rowk = index i8 %b, %scaled
    %q = index f64 %rowk, %j
    %w = load typed f64 %q
    %sum = binary add f64 %s, %w
    %k1 = binary add i64 %k, 1
    %c = cmp lt i64 %k1, 32
    branch %c, ^terms, ^next
  block ^next:
    store typed f64 %sum, %cell
    %j1 = binary add i64 %j, 1
    %d = cmp lt i64 %j1, 32
    branch %d, ^columns, ^done
  block ^done:
    return void
}

# The first store is not of the sum's start: the loops stay.
function @other_first() -> void [no_inline=yes] {
  block ^entry:
    %r = addr @out
    %b = addr @right
    jump ^columns
  block ^columns:
    %j = phi i64 [^entry: 0, ^next: %j1]
    %cell = index f64 %r, %j
    store typed f64 1, %cell
    jump ^terms
  block ^terms:
    %k = phi i64 [^columns: 0, ^terms: %k1]
    %s = phi f64 [^columns: 0, ^terms: %sum]
    %scaled = binary mul i64 %k, 256
    %rowk = index i8 %b, %scaled
    %q = index f64 %rowk, %j
    %w = load typed f64 %q
    %sum = binary add f64 %s, %w
    %k1 = binary add i64 %k, 1
    %c = cmp lt i64 %k1, 32
    branch %c, ^terms, ^next
  block ^next:
    store typed f64 %sum, %cell
    %j1 = binary add i64 %j, 1
    %d = cmp lt i64 %j1, 32
    branch %d, ^columns, ^done
  block ^done:
    return void
}

# The outer loop's branch reads the inner counter: the loops stay.
function @reads_inner() -> void [no_inline=yes] {
  block ^entry:
    %r = addr @out
    %b = addr @right
    jump ^columns
  block ^columns:
    %j = phi i64 [^entry: 0, ^next: %j1]
    %cell = index f64 %r, %j
    jump ^terms
  block ^terms:
    %k = phi i64 [^columns: 0, ^terms: %k1]
    %s = phi f64 [^columns: 0, ^terms: %sum]
    %scaled = binary mul i64 %k, 256
    %rowk = index i8 %b, %scaled
    %q = index f64 %rowk, %j
    %w = load typed f64 %q
    %sum = binary add f64 %s, %w
    %k1 = binary add i64 %k, 1
    %c = cmp lt i64 %k1, 32
    branch %c, ^terms, ^next
  block ^next:
    store typed f64 %sum, %cell
    %j1 = binary add i64 %j, 1
    %e = binary add i64 %k1, %j
    %d = cmp lt i64 %e, 60
    branch %d, ^columns, ^done
  block ^done:
    return void
}

# The read steps by two elements with the outer counter: the loops stay.
function @wide_read() -> void [no_inline=yes] {
  block ^entry:
    %r = addr @out
    %b = addr @right
    jump ^columns
  block ^columns:
    %j = phi i64 [^entry: 0, ^next: %j1]
    %cell = index f64 %r, %j
    %twice = binary mul i64 %j, 2
    jump ^terms
  block ^terms:
    %k = phi i64 [^columns: 0, ^terms: %k1]
    %s = phi f64 [^columns: 0, ^terms: %sum]
    %scaled = binary mul i64 %k, 256
    %rowk = index i8 %b, %scaled
    %q = index f64 %rowk, %twice
    %w = load typed f64 %q
    %sum = binary add f64 %s, %w
    %k1 = binary add i64 %k, 1
    %c = cmp lt i64 %k1, 32
    branch %c, ^terms, ^next
  block ^next:
    store typed f64 %sum, %cell
    %j1 = binary add i64 %j, 1
    %d = cmp lt i64 %j1, 32
    branch %d, ^columns, ^done
  block ^done:
    return void
}

# The cell steps by two elements: the loops stay.
function @wide_cell() -> void [no_inline=yes] {
  block ^entry:
    %r = addr @out
    %b = addr @right
    jump ^columns
  block ^columns:
    %j = phi i64 [^entry: 0, ^next: %j1]
    %twice = binary mul i64 %j, 2
    %cell = index f64 %r, %twice
    jump ^terms
  block ^terms:
    %k = phi i64 [^columns: 0, ^terms: %k1]
    %s = phi f64 [^columns: 0, ^terms: %sum]
    %scaled = binary mul i64 %k, 256
    %rowk = index i8 %b, %scaled
    %q = index f64 %rowk, %j
    %w = load typed f64 %q
    %sum = binary add f64 %s, %w
    %k1 = binary add i64 %k, 1
    %c = cmp lt i64 %k1, 32
    branch %c, ^terms, ^next
  block ^next:
    store typed f64 %sum, %cell
    %j1 = binary add i64 %j, 1
    %d = cmp lt i64 %j1, 32
    branch %d, ^columns, ^done
  block ^done:
    return void
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}

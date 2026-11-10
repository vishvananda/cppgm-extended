# The literal 0 is the zero vector in every position a v128 operand takes.
global @memory : obj<16x16> = {
  i64 -1
  i64 -1
}

function @chosen(%flag : i32) -> v128 {
  block ^entry:
    branch %flag, ^five, ^join

  block ^five:
    %value = vector fromi32 5
    jump ^join

  block ^join:
    %result = phi v128 [^entry: 0, ^five: %value]
    return v128 %result
}

function @nothing() -> v128 {
  block ^entry:
    return v128 0
}

function @sum(%a : v128, %b : v128) -> i32 {
  block ^entry:
    %total = vector paddd %a, %b
    %low = vector toi32 %total
    return i32 %low
}

function @ninth(%a : v128, %b : v128, %c : v128, %d : v128, %e : v128, %f : v128, %g : v128, %h : v128, %i : v128) -> i32 {
  block ^entry:
    %low = vector toi32 %i
    return i32 %low
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %place = addr @memory
    store v128 0, %place
    %first = load i64 %place
    %next = index i64 %place, 1
    %second = load i64 %next
    %stored = binary or i64 %first, %second
    %stored_zero = cmp eq i64 %stored, 0
    branch %stored_zero, ^returned, ^bad_store

  block ^bad_store:
    return i32 1

  block ^returned:
    %none = call v128 @nothing()
    %low = vector toi64 %none
    %returned_zero = cmp eq i64 %low, 0
    branch %returned_zero, ^passed, ^bad_return

  block ^bad_return:
    return i32 2

  block ^passed:
    %seven = vector fromi32 7
    %left = call i32 @sum(0, %seven)
    %right = call i32 @sum(%seven, 0)
    %both = binary add i32 %left, %right
    %fourteen = cmp eq i32 %both, 14
    branch %fourteen, ^joined, ^bad_argument

  block ^bad_argument:
    return i32 3

  block ^joined:
    %off = call v128 @chosen(0)
    %on = call v128 @chosen(1)
    %off_low = vector toi32 %off
    %on_low = vector toi32 %on
    %five = binary add i32 %off_low, %on_low
    %right_join = cmp eq i32 %five, 5
    branch %right_join, ^operand, ^bad_join

  block ^bad_join:
    return i32 4

  block ^operand:
    %kept = vector paddd %seven, 0
    %kept_low = vector toi32 %kept
    %still = cmp eq i32 %kept_low, 7
    branch %still, ^stacked, ^bad_operand

  block ^bad_operand:
    return i32 5

  block ^stacked:
    %empty = call i32 @ninth(%seven, %seven, %seven, %seven, %seven, %seven, %seven, %seven, 0)
    %on_stack = call i32 @ninth(0, 0, 0, 0, 0, 0, 0, 0, %seven)
    %mismatch = binary sub i32 %on_stack, 7
    %wrong = binary or i32 %empty, %mismatch
    branch %wrong, ^bad_stack, ^done

  block ^bad_stack:
    return i32 6

  block ^done:
    return i32 0
}

# Slot values are read where used, rather than once for the whole function.
function @identity(%value : i64) -> i64 [no_inline=yes, unwind=no] {
  block ^entry:
    return i64 %value
}

function @mutate(%address : ptr [pass=by_address]) -> void [no_inline=yes, unwind=no] {
  block ^entry:
    store i64 29, %address
    return void
}

function @main() -> i64 [role=entry] {
  slot $source : i64
  slot $destination : i64
  slot $condition : i64
  slot $offset : i64
  slot $shift_count : i64
  slot $cells : obj<24x8>
  slot $floating : f64

  block ^entry:
    store i64 11, $source
    store i64 $source, $destination
    %before = binary add i64 $source, 3
    store i64 17, $source
    %after = binary add i64 $source, 3
    %delta = binary sub i64 %after, %before
    %stored = load i64 $destination
    %bad_store = cmp ne i64 %stored, 11
    %bad_delta = cmp ne i64 %delta, 6
    %copy = copy i64 $source
    %negated = unary neg i64 $source
    store i64 23, $source
    %bad_copy = cmp ne i64 %copy, 17
    %bad_negated = cmp ne i64 %negated, -17
    %compared = cmp eq i64 $source, 23
    %converted = convert trunc i32 i64 $source
    %called = call i64 @identity($source)
    call void @mutate($source)
    %changed = call i64 @identity($source)
    %bad_compare = cmp ne i64 %compared, 1
    %bad_convert = cmp ne i32 %converted, 23
    %bad_call = cmp ne i64 %called, 23
    %bad_change = cmp ne i64 %changed, 29
    store f64 1.5, $floating
    %sum = binary add f64 $floating, 0.5
    store f64 9.0, $floating
    %bad_float = cmp ne f64 %sum, 2.0
    store i64 1, $condition
    %selected = select i64 $condition, $source, $destination
    %bad_select = cmp ne i64 %selected, 29
    %cells = addr $cells
    store i64 2, $offset
    %cell = index i64 %cells, $offset
    store i64 37, %cell
    store i64 1, $offset
    %other_cell = index i64 %cells, $offset
    store i64 41, %other_cell
    %indexed = load i64 %cell
    %bad_index = cmp ne i64 %indexed, 37
    store i64 5, $shift_count
    %wide = binary shl i128 1, $shift_count
    store i64 7, $shift_count
    %shifted = convert trunc i64 i128 %wide
    %bad_shift = cmp ne i64 %shifted, 32
    %bad0 = binary or i64 %bad_store, %bad_delta
    %bad1 = binary or i64 %bad_copy, %bad_negated
    %bad2 = binary or i64 %bad_compare, %bad_convert
    %bad3 = binary or i64 %bad_call, %bad_change
    %bad4 = binary or i64 %bad_float, %bad_select
    %bad5 = binary or i64 %bad0, %bad1
    %bad6 = binary or i64 %bad2, %bad3
    %bad7 = binary or i64 %bad4, %bad_index
    %bad8 = binary or i64 %bad5, %bad6
    %bad9 = binary or i64 %bad8, %bad7
    %bad = binary or i64 %bad9, %bad_shift
    branch $condition, ^chosen, ^wrong

  block ^chosen:
    switch $source, ^wrong, 29:^done

  block ^done:
    store i64 %bad, $destination
    return i64 $destination

  block ^wrong:
    return i64 1
}

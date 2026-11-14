# Two 32-bit members at adjacent places stored one after the other are one
# 64-bit store: of the 64-bit value whose halves they are (@put), or of one
# 64-bit load when they are loads of two adjacent members with no write
# between the two loads (@move, @move_down in the other order). The 64-bit
# load stands after the later of the two loads: in @exchange each object is
# read once and written once. In @rotate the second member is loaded after
# the first is stored, in @written_between the first member is written
# between the loads, and in @apart the places are not adjacent: all three
# keep the narrow stores.
global @cells : obj<48x8> = { zero 48 }

function @put(%to : ptr, %i : i64, %value : i64) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %at = index i64 %to, %i
    %low = convert trunc i32 i64 %value
    store typed i32 %low, %at
    %top = binary ushr i64 %value, 32
    %high = convert trunc i32 i64 %top
    %at4 = index i8 %at, 4
    store typed i32 %high, %at4
    return void
}

function @move(%p : ptr, %i : i64, %j : i64) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %from = index i64 %p, %j
    %to = index i64 %p, %i
    %a = load typed i32 %from
    %from4 = index i8 %from, 4
    %b = load typed i32 %from4
    store typed i32 %a, %to
    %to4 = index i8 %to, 4
    store typed i32 %b, %to4
    return void
}

function @move_down(%p : ptr, %i : i64, %j : i64) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %from = index i64 %p, %j
    %to = index i64 %p, %i
    %from4 = index i8 %from, 4
    %b = load typed i32 %from4
    %a = load typed i32 %from
    %to4 = index i8 %to, 4
    store typed i32 %b, %to4
    store typed i32 %a, %to
    return void
}

function @rotate(%p : ptr, %q : ptr) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %a = load i32 %p
    store i32 %a, %q
    %p4 = index i8 %p, 4
    %b = load i32 %p4
    %q4 = index i8 %q, 4
    store i32 %b, %q4
    return void
}

function @exchange(%p : ptr, %q : ptr) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %a = load i32 %p
    %p4 = index i8 %p, 4
    %b = load i32 %p4
    %c = load i32 %q
    %q4 = index i8 %q, 4
    %d = load i32 %q4
    store i32 %c, %p
    store i32 %d, %p4
    store i32 %a, %q
    store i32 %b, %q4
    return void
}

function @written_between(%p : ptr, %q : ptr) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %a = load i32 %p
    store i32 9, %p
    %p4 = index i8 %p, 4
    %b = load i32 %p4
    store i32 %a, %q
    %q4 = index i8 %q, 4
    store i32 %b, %q4
    return void
}

function @apart(%p : ptr, %q : ptr) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %a = load i32 %p
    %p4 = index i8 %p, 4
    %b = load i32 %p4
    store i32 %a, %q
    %q8 = index i8 %q, 8
    store i32 %b, %q8
    return void
}

function @main() -> i64 [role=entry] {
  block ^entry:
    %cells = addr @cells
    call void @put(%cells, 0, 8589934593)
    call void @move(%cells, 1, 0)
    call void @move_down(%cells, 2, 1)
    %c2 = index i64 %cells, 2
    %moved = load i64 %c2
    %moved_bad = cmp ne i64 %moved, 8589934593
    %c4 = index i8 %cells, 4
    call void @rotate(%cells, %c4)
    %first = load i64 %cells
    %first_bad = cmp ne i64 %first, 4294967297
    %third = index i8 %cells, 8
    %third_value = load i32 %third
    %third_bad = cmp ne i32 %third_value, 1
    %c3 = index i64 %cells, 3
    call void @apart(%c2, %c3)
    %fourth = load i64 %c3
    %fourth_bad = cmp ne i64 %fourth, 1
    %c3b = index i8 %c3, 8
    %fifth = load i32 %c3b
    %fifth_bad = cmp ne i32 %fifth, 2
    call void @exchange(%cells, %c3)
    %sixth = load i64 %cells
    %sixth_bad = cmp ne i64 %sixth, 1
    %seventh = load i64 %c3
    %seventh_bad = cmp ne i64 %seventh, 4294967297
    %c5 = index i64 %cells, 5
    call void @written_between(%c2, %c5)
    %eighth = load i64 %c5
    %eighth_bad = cmp ne i64 %eighth, 8589934593
    %ninth = load i64 %c2
    %ninth_bad = cmp ne i64 %ninth, 8589934601
    %bad0 = binary or i64 %moved_bad, %first_bad
    %bad1 = binary or i64 %third_bad, %fourth_bad
    %bad2 = binary or i64 %fifth_bad, %sixth_bad
    %bad3 = binary or i64 %seventh_bad, %eighth_bad
    %bad4 = binary or i64 %bad0, %bad1
    %bad5 = binary or i64 %bad2, %bad3
    %bad6 = binary or i64 %bad4, %bad5
    %bad = binary or i64 %bad6, %ninth_bad
    return i64 %bad
}

function @element_null(%base : ptr, %offset : i64) -> i1 [no_inline=yes] {
  block ^entry:
    %p = index inbounds i32 [projection=array_element] %base, %offset
    %null = cmp eq ptr %p, 0
    return i1 %null
}

function @element_nonnull(%base : ptr, %offset : i64) -> i1 [no_inline=yes] {
  block ^entry:
    %p = index inbounds i32 %base, %offset
    %nonnull = cmp ne ptr 0, %p
    return i1 %nonnull
}

function @wrapping(%base : ptr, %offset : i64) -> i1 [no_inline=yes] {
  block ^entry:
    %p = index i8 [projection=array_element] %base, %offset
    %null = cmp eq ptr %p, 0
    return i1 %null
}

function @mixed(%base : ptr, %offset : i64) -> i1 [no_inline=yes] {
  block ^entry:
    %first = index i8 %base, %offset
    %p = index inbounds i8 %first, 0
    %null = cmp eq ptr %p, 0
    return i1 %null
}

# The fact holds on the selected path only. In particular, null with an
# unselected nonzero offset must not turn the result into a non-null value.
function @conditional(%flag : i1, %base : ptr, %offset : i64) -> i1 [no_inline=yes] {
  slot $value : ptr
  block ^entry:
    branch %flag, ^yes, ^no
  block ^yes:
    %p = index inbounds i32 %base, %offset
    store ptr %p, $value
    jump ^join
  block ^no:
    store ptr %base, $value
    jump ^join
  block ^join:
    %result = load ptr $value
    %null = cmp eq ptr %result, 0
    return i1 %null
}

function @main() -> i32 {
  slot $cells : obj<16x4>
  block ^entry:
    %base = addr $cells
    %end = index inbounds i32 %base, 4
    %middle = index inbounds i32 %end, -2
    store volatile i32 29, %middle
    %observed = load volatile i32 %middle
    %bad_value = cmp ne i32 %observed, 29
    %one_past = call i1 @element_nonnull(%base, 4)
    %negative = call i1 @element_nonnull(%end, -2)
    %null_zero = call i1 @element_null(0, 0)
    %raw = call i1 @wrapping(1024, -1024)
    %chain = call i1 @mixed(2048, -2048)
    %skipped = call i1 @conditional(0, 0, 3)
    %selected = call i1 @conditional(1, %base, 2)
    %a = binary and i64 %one_past, %negative
    %b = binary and i64 %null_zero, %raw
    %c = binary and i64 %chain, %skipped
    %ab = binary and i64 %a, %b
    %ok = binary and i64 %ab, %c
    %bad = cmp ne i64 %ok, 1
    %bad0 = binary or i64 %bad, %bad_value
    %bad1 = binary or i64 %bad0, %selected
    %status = convert trunc i32 i64 %bad1
    return i32 %status
}

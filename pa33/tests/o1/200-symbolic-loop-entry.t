# Equivalent projections and a phi whose value stays live after an empty loop.
# Its volatile body must disappear only when the entry test proves zero trips.
function @empty(%p : ptr, %hits : ptr) -> ptr [no_inline=yes] {
  block ^entry:
    %begin = index i8 [projection=field] %p, 24
    %end = index i8 %p, 24
    jump ^test
  block ^test:
    %it = phi ptr [^entry: %begin, ^body: %next]
    %more = cmp ne ptr %it, %end
    branch %more, ^body, ^exit
  block ^body:
    store volatile i64 91, %hits
    %next = index i8 %it, 8
    jump ^test
  block ^exit:
    return ptr %it
}
function @nonempty(%p : ptr, %hits : ptr) -> ptr [no_inline=yes] {
  block ^entry:
    %begin = index i8 [projection=field] %p, 24
    %end = index i8 %p, 32
    jump ^test
  block ^test:
    %it = phi ptr [^entry: %begin, ^body: %next]
    %more = cmp ne ptr %it, %end
    branch %more, ^body, ^exit
  block ^body:
    store volatile i64 13, %hits
    %next = index i8 %it, 8
    jump ^test
  block ^exit:
    return ptr %it
}
function @main() -> i64 [role=entry] {
  slot $array : obj<64x8>
  slot $hits : i64
  block ^entry:
    %p = addr $array
    %h = addr $hits
    store volatile i64 0, %h
    %a = call ptr @empty(%p, %h)
    %ha = load volatile i64 %h
    %b = call ptr @nonempty(%p, %h)
    %hb = load volatile i64 %h
    %expected_a = index i8 %p, 24
    %expected_b = index i8 %p, 32
    %bad_a = cmp ne ptr %a, %expected_a
    %bad_b = cmp ne ptr %b, %expected_b
    %bad_ha = cmp ne i64 %ha, 0
    %bad_hb = cmp ne i64 %hb, 13
    %x = binary or i64 %bad_a, %bad_b
    %y = binary or i64 %bad_ha, %bad_hb
    %bad = binary or i64 %x, %y
    return i64 %bad
}

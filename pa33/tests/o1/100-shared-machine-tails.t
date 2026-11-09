global @storage : obj<512x8> = zero
function @shared_cases(%p : ptr, %kind : i64) -> ptr [no_inline=yes] {
  block ^entry:
    switch %kind, ^fallback, 0: ^case0, 1: ^case1, 2: ^case2, 3: ^case3, 4: ^case4, 5: ^case5, 6: ^case6, 7: ^case7, 8: ^case8, 9: ^case9, 10: ^case10, 11: ^case11
  block ^case0:
    %p0 = index i8 %p, -40
    return ptr %p0
  block ^case1:
    %p1 = index i8 %p, -48
    return ptr %p1
  block ^case2:
    %p2 = index i8 %p, -56
    return ptr %p2
  block ^case3:
    %p3 = index i8 %p, -40
    return ptr %p3
  block ^case4:
    %p4 = index i8 %p, -48
    return ptr %p4
  block ^case5:
    %p5 = index i8 %p, -56
    return ptr %p5
  block ^case6:
    %p6 = index i8 %p, -40
    return ptr %p6
  block ^case7:
    %p7 = index i8 %p, -48
    return ptr %p7
  block ^case8:
    %p8 = index i8 %p, -56
    return ptr %p8
  block ^case9:
    %p9 = index i8 %p, -40
    return ptr %p9
  block ^case10:
    %p10 = index i8 %p, -48
    return ptr %p10
  block ^case11:
    %p11 = index i8 %p, -56
    return ptr %p11
  block ^fallback:
    return ptr %p
}
function @distinct_cases(%p : ptr, %kind : i64) -> ptr [no_inline=yes] {
  block ^entry:
    switch %kind, ^fallback, 0: ^case0, 1: ^case1, 2: ^case2, 3: ^case3, 4: ^case4, 5: ^case5, 6: ^case6, 7: ^case7, 8: ^case8, 9: ^case9, 10: ^case10, 11: ^case11
  block ^case0:
    %p0 = index i8 %p, -40
    return ptr %p0
  block ^case1:
    %p1 = index i8 %p, -48
    return ptr %p1
  block ^case2:
    %p2 = index i8 %p, -56
    return ptr %p2
  block ^case3:
    %p3 = index i8 %p, -64
    return ptr %p3
  block ^case4:
    %p4 = index i8 %p, -72
    return ptr %p4
  block ^case5:
    %p5 = index i8 %p, -80
    return ptr %p5
  block ^case6:
    %p6 = index i8 %p, -88
    return ptr %p6
  block ^case7:
    %p7 = index i8 %p, -96
    return ptr %p7
  block ^case8:
    %p8 = index i8 %p, -104
    return ptr %p8
  block ^case9:
    %p9 = index i8 %p, -112
    return ptr %p9
  block ^case10:
    %p10 = index i8 %p, -120
    return ptr %p10
  block ^case11:
    %p11 = index i8 %p, -128
    return ptr %p11
  block ^fallback:
    return ptr %p
}
function @shared_store_paths(%p : ptr, %kind : i64) -> i64 [no_inline=yes] {
  block ^entry:
    switch %kind, ^fallback, 0: ^case0, 1: ^case1, 2: ^case2, 3: ^case3, 4: ^case4, 5: ^case5, 6: ^case6, 7: ^case7, 8: ^case8, 9: ^case9, 10: ^case10, 11: ^case11
  block ^case0:
    store volatile i64 1, %p
    jump ^link0
  block ^case1:
    store volatile i64 2, %p
    jump ^link1
  block ^case2:
    store volatile i64 3, %p
    jump ^link2
  block ^case3:
    store volatile i64 1, %p
    jump ^link3
  block ^case4:
    store volatile i64 2, %p
    jump ^link4
  block ^case5:
    store volatile i64 3, %p
    jump ^link5
  block ^case6:
    store volatile i64 1, %p
    jump ^link6
  block ^case7:
    store volatile i64 2, %p
    jump ^link7
  block ^case8:
    store volatile i64 3, %p
    jump ^link8
  block ^case9:
    store volatile i64 1, %p
    jump ^link9
  block ^case10:
    store volatile i64 2, %p
    jump ^link10
  block ^case11:
    store volatile i64 3, %p
    jump ^link11
  block ^fallback:
    store volatile i64 0, %p
    jump ^done
  block ^link0:
    jump ^done
  block ^link1:
    jump ^done
  block ^link2:
    jump ^done
  block ^link3:
    jump ^done
  block ^link4:
    jump ^done
  block ^link5:
    jump ^done
  block ^link6:
    jump ^done
  block ^link7:
    jump ^done
  block ^link8:
    jump ^done
  block ^link9:
    jump ^done
  block ^link10:
    jump ^done
  block ^link11:
    jump ^done
  block ^done:
    return i64 0
}
function @main() -> i64 [role=entry] {
  block ^entry:
    %base = addr @storage
    %p = index i8 %base, 256
    jump ^loop
  block ^loop:
    %i = phi i64 [^entry: 0, ^next: %inext]
    %a = call ptr @shared_cases(%p, %i)
    %b = call ptr @distinct_cases(%p, %i)
    %stored = call i64 @shared_store_paths(%p, %i)
    %observed = load volatile i64 %p
    %inside = cmp ult i64 %i, 12
    %mod = binary umod i64 %i, 3
    %step = binary add i64 %mod, 1
    %expected_store = select i64 %inside, %step, 0
    %bad_store = cmp ne i64 %observed, %expected_store
    %widtha = binary mul i64 %mod, 8
    %widthb = binary mul i64 %i, 8
    %offseta = binary sub i64 -40, %widtha
    %offsetb = binary sub i64 -40, %widthb
    %pa = index i8 %p, %offseta
    %pb = index i8 %p, %offsetb
    %expected_a = select ptr %inside, %pa, %p
    %expected_b = select ptr %inside, %pb, %p
    %bad_a = cmp ne ptr %a, %expected_a
    %bad_b = cmp ne ptr %b, %expected_b
    %bad_ptr = binary or i64 %bad_a, %bad_b
    %bad = binary or i64 %bad_ptr, %bad_store
    branch %bad, ^fail, ^next
  block ^next:
    %inext = binary add i64 %i, 1
    %more = cmp ne i64 %inext, 16
    branch %more, ^loop, ^done
  block ^fail:
    return i64 1
  block ^done:
    return i64 0
}

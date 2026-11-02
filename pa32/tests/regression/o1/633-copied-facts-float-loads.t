# What a copy's source is known to hold, its destination holds after the
# copy: @copied's f64 loads from $b read the eightbyte literal stored in $a
# (its bits as a double), the parameter stored there, and the zeroed tail.
# @overwritten's store into $b after the copy replaces what it carried.
function @copied(%x : f64) -> f64 [no_inline=yes] {
  slot $a : obj<24x8>
  slot $b : obj<24x8>

  block ^entry:
    %pa = addr $a
    store i64 4614500768194494464, %pa
    %fa = index i8 %pa, 8
    store f64 %x, %fa
    %za = index i8 %pa, 16
    store i64 0, %za
    %pb = addr $b
    copyobj 24x8 %pa, %pb
    %l0 = load f64 %pb
    %fb = index i8 %pb, 8
    %l1 = load f64 %fb
    %zb = index i8 %pb, 16
    %l2 = load f64 %zb
    call void @escape(%pa, %pb)
    %s = binary add f64 %l0, %l1
    %t = binary add f64 %s, %l2
    return f64 %t
}

function @overwritten(%x : f64, %y : f64) -> f64 [no_inline=yes] {
  slot $a : obj<16x8>
  slot $b : obj<16x8>

  block ^entry:
    %pa = addr $a
    store f64 %x, %pa
    %fa = index i8 %pa, 8
    store f64 %x, %fa
    %pb = addr $b
    copyobj 16x8 %pa, %pb
    store f64 %y, %pb
    %l0 = load f64 %pb
    %fb = index i8 %pb, 8
    %l1 = load f64 %fb
    call void @escape(%pa, %pb)
    %s = binary add f64 %l0, %l1
    return f64 %s
}

function @escape(%a : ptr, %b : ptr) -> void [no_inline=yes] {
  block ^entry:
    return void
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %c = call f64 @copied(1.5)
    %o = call f64 @overwritten(1, 2)
    %s = binary add f64 %c, %o
    %bad = cmp ne f64 %s, 7.75
    %r = convert zext i32 i1 %bad
    return i32 %r
}

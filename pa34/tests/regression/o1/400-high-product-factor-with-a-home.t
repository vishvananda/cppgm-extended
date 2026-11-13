# The high half of a 64-bit product reads its factors at the truncation,
# after the multiply loads one factor into rax. @wide's factor %b is also
# read in ^more, and the values live across ^entry leave it a frame home.
# Its register in ^entry still holds it at the truncation, so it is neither
# rax nor rdx: the product is that of %w and %b, not of %w and itself.
# @main returns the low byte of @wide's result (195).
global @table : obj<104x8> = { zero 104 }

function @wide(%w : i64, %p : ptr, %f : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %e0 = index i64 %p, 1
    %v0 = load i64 %e0
    %e1 = index i64 %p, 2
    %v1 = load i64 %e1
    %e2 = index i64 %p, 3
    %v2 = load i64 %e2
    %e3 = index i64 %p, 4
    %v3 = load i64 %e3
    %e4 = index i64 %p, 5
    %v4 = load i64 %e4
    %e5 = index i64 %p, 6
    %v5 = load i64 %e5
    %e6 = index i64 %p, 7
    %v6 = load i64 %e6
    %e7 = index i64 %p, 8
    %v7 = load i64 %e7
    %e8 = index i64 %p, 9
    %v8 = load i64 %e8
    %e9 = index i64 %p, 10
    %v9 = load i64 %e9
    %e10 = index i64 %p, 11
    %v10 = load i64 %e10
    %e11 = index i64 %p, 12
    %v11 = load i64 %e11
    %b = load i64 %p
    %x = convert zext i128 i64 %w
    %y = convert zext i128 i64 %b
    %m = binary mul i128 %x, %y
    %h = binary ushr i128 %m, 64
    %r = convert trunc i64 i128 %h
    %go = cmp ne i64 %f, 0
    branch %go, ^more, ^done

  block ^more:
    %lo = binary mul i64 %w, %b
    %s0 = binary add i64 %lo, %v0
    %s1 = binary add i64 %s0, %v1
    %s2 = binary add i64 %s1, %v2
    %s3 = binary add i64 %s2, %v3
    %s4 = binary add i64 %s3, %v4
    %s5 = binary add i64 %s4, %v5
    %s6 = binary add i64 %s5, %v6
    %s7 = binary add i64 %s6, %v7
    %s8 = binary add i64 %s7, %v8
    %s9 = binary add i64 %s8, %v9
    %s10 = binary add i64 %s9, %v10
    %s11 = binary add i64 %s10, %v11
    %t0 = binary xor i64 %s11, %v0
    %t1 = binary xor i64 %t0, %v1
    %t2 = binary xor i64 %t1, %v2
    %t3 = binary xor i64 %t2, %v3
    %t4 = binary xor i64 %t3, %v4
    %t5 = binary xor i64 %t4, %v5
    %t6 = binary xor i64 %t5, %v6
    %t7 = binary xor i64 %t6, %v7
    %t8 = binary xor i64 %t7, %v8
    %t9 = binary xor i64 %t8, %v9
    %t10 = binary xor i64 %t9, %v10
    %t11 = binary xor i64 %t10, %v11
    %u = binary add i64 %t11, %r
    jump ^done

  block ^done:
    %q = phi i64 [^entry: %r, ^more: %u]
    %z0 = binary add i64 %q, %v0
    %z1 = binary add i64 %z0, %v1
    %z2 = binary add i64 %z1, %v2
    %z3 = binary add i64 %z2, %v3
    %z4 = binary add i64 %z3, %v4
    %z5 = binary add i64 %z4, %v5
    %z6 = binary add i64 %z5, %v6
    %z7 = binary add i64 %z6, %v7
    %z8 = binary add i64 %z7, %v8
    %z9 = binary add i64 %z8, %v9
    %z10 = binary add i64 %z9, %v10
    %z11 = binary add i64 %z10, %v11
    return i64 %z11
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %p = addr @table
    %a0 = index i64 %p, 0
    store i64 6, %a0
    %a1 = index i64 %p, 1
    store i64 13, %a1
    %a2 = index i64 %p, 2
    store i64 20, %a2
    %a3 = index i64 %p, 3
    store i64 27, %a3
    %a4 = index i64 %p, 4
    store i64 34, %a4
    %a5 = index i64 %p, 5
    store i64 41, %a5
    %a6 = index i64 %p, 6
    store i64 48, %a6
    %a7 = index i64 %p, 7
    store i64 55, %a7
    %a8 = index i64 %p, 8
    store i64 62, %a8
    %a9 = index i64 %p, 9
    store i64 69, %a9
    %a10 = index i64 %p, 10
    store i64 76, %a10
    %a11 = index i64 %p, 11
    store i64 83, %a11
    %a12 = index i64 %p, 12
    store i64 90, %a12
    %v = call i64 @wide(-1, %p, 1)
    %status = convert trunc i32 i64 %v
    return i32 %status
}

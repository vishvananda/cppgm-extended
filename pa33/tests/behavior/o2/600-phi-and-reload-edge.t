# Check defined values and live ranges under any valid instruction selection.
function @walk(%p : ptr, %n : u32, %first : u32) -> u32 [no_inline=yes] {
  block ^entry:
    jump ^head

  block ^head:
    %w = phi u32 [^entry: %n, ^next: %w1]
    %t = phi i64 [^entry: 0, ^next: %u]
    %c = cmp ugt u32 %w, %first
    branch %c, ^body, ^done

  block ^body:
    %w1 = binary sub u32 %w, 1
    jump ^inner

  block ^inner:
    %k = phi i64 [^body: 0, ^next: %k1]
    %a0 = phi i64 [^body: 0, ^next: %b0]
    %a1 = phi i64 [^body: 1, ^next: %b1]
    %a2 = phi i64 [^body: 2, ^next: %b2]
    %a3 = phi i64 [^body: 3, ^next: %b3]
    %a4 = phi i64 [^body: 4, ^next: %b4]
    %a5 = phi i64 [^body: 5, ^next: %b5]
    %a6 = phi i64 [^body: 6, ^next: %b6]
    %a7 = phi i64 [^body: 7, ^next: %b7]
    %a8 = phi i64 [^body: 8, ^next: %b8]
    %a9 = phi i64 [^body: 9, ^next: %b9]
    %a10 = phi i64 [^body: 10, ^next: %b10]
    %a11 = phi i64 [^body: 11, ^next: %b11]
    %a12 = phi i64 [^body: 12, ^next: %b12]
    %a13 = phi i64 [^body: 13, ^next: %b13]
    %a14 = phi i64 [^body: 14, ^next: %b14]
    %q = index i64 %p, %k
    %v = load i64 %q
    %x0 = binary add i64 %k, 1
    %y0 = binary mul i64 %a0, %x0
    %b0 = binary xor i64 %y0, %v
    %x1 = binary add i64 %k, 4
    %y1 = binary mul i64 %a1, %x1
    %b1 = binary xor i64 %y1, %v
    %x2 = binary add i64 %k, 7
    %y2 = binary mul i64 %a2, %x2
    %b2 = binary xor i64 %y2, %v
    %x3 = binary add i64 %k, 10
    %y3 = binary mul i64 %a3, %x3
    %b3 = binary xor i64 %y3, %v
    %x4 = binary add i64 %k, 13
    %y4 = binary mul i64 %a4, %x4
    %b4 = binary xor i64 %y4, %v
    %x5 = binary add i64 %k, 16
    %y5 = binary mul i64 %a5, %x5
    %b5 = binary xor i64 %y5, %v
    %x6 = binary add i64 %k, 19
    %y6 = binary mul i64 %a6, %x6
    %b6 = binary xor i64 %y6, %v
    %x7 = binary add i64 %k, 22
    %y7 = binary mul i64 %a7, %x7
    %b7 = binary xor i64 %y7, %v
    %x8 = binary add i64 %k, 25
    %y8 = binary mul i64 %a8, %x8
    %b8 = binary xor i64 %y8, %v
    %x9 = binary add i64 %k, 28
    %y9 = binary mul i64 %a9, %x9
    %b9 = binary xor i64 %y9, %v
    %x10 = binary add i64 %k, 31
    %y10 = binary mul i64 %a10, %x10
    %b10 = binary xor i64 %y10, %v
    %x11 = binary add i64 %k, 34
    %y11 = binary mul i64 %a11, %x11
    %b11 = binary xor i64 %y11, %v
    %x12 = binary add i64 %k, 37
    %y12 = binary mul i64 %a12, %x12
    %b12 = binary xor i64 %y12, %v
    %x13 = binary add i64 %k, 40
    %y13 = binary mul i64 %a13, %x13
    %b13 = binary xor i64 %y13, %v
    %x14 = binary add i64 %k, 43
    %y14 = binary mul i64 %a14, %x14
    %b14 = binary xor i64 %y14, %v
    %k1 = binary add i64 %k, 1
    %hit = cmp eq i64 %v, 99
    branch %hit, ^found, ^next

  block ^next:
    %s0 = copy i64 %b0
    %s1 = binary add i64 %s0, %b1
    %s2 = binary add i64 %s1, %b2
    %s3 = binary add i64 %s2, %b3
    %s4 = binary add i64 %s3, %b4
    %s5 = binary add i64 %s4, %b5
    %s6 = binary add i64 %s5, %b6
    %s7 = binary add i64 %s6, %b7
    %s8 = binary add i64 %s7, %b8
    %s9 = binary add i64 %s8, %b9
    %s10 = binary add i64 %s9, %b10
    %s11 = binary add i64 %s10, %b11
    %s12 = binary add i64 %s11, %b12
    %s13 = binary add i64 %s12, %b13
    %s14 = binary add i64 %s13, %b14
    %u = binary add i64 %t, %s14
    %more = cmp lt i64 %k1, 4
    branch %more, ^inner, ^head

  block ^found:
    %r = binary add u32 %w, 1000
    return u32 %r

  block ^done:
    %tt = convert trunc u32 i64 %t
    %d = binary and u32 %tt, 255
    return u32 %d
}

function @main() -> i32 [role=entry] {
  slot $data : obj<32x8>
  block ^entry:
    %p = addr $data
    store i64 1, %p
    %p1 = index i64 %p, 1
    store i64 2, %p1
    %p2 = index i64 %p, 2
    store i64 3, %p2
    %p3 = index i64 %p, 3
    store i64 4, %p3
    %a = call u32 @walk(%p, 6, 0)
    store i64 99, %p2
    %b = call u32 @walk(%p, 6, 0)
    %s = binary add u32 %a, %b
    %s64 = convert zext i64 u32 %s
    %r = convert trunc i32 i64 %s64
    %m = binary and i32 %r, 127
    return i32 %m
}

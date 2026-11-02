# Each loop of one function goes, however many it has: removing one takes a
# pass over the function, and a fixed number of passes would leave the rest.
function @many(%n : i32) -> i32 [binding=strong] {
  block ^entry:
    jump ^h0

  block ^h0:
    %i0 = phi i32 [^entry: 0, ^b0: %j0]
    %s0 = phi i32 [^entry: 0, ^b0: %t0]
    %c0 = cmp lt i32 %i0, 1000
    branch %c0, ^b0, ^h1

  block ^b0:
    %t0 = binary add i32 %s0, 1
    %j0 = binary add i32 %i0, 1
    jump ^h0

  block ^h1:
    %i1 = phi i32 [^h0: 0, ^b1: %j1]
    %s1 = phi i32 [^h0: %s0, ^b1: %t1]
    %c1 = cmp lt i32 %i1, 1000
    branch %c1, ^b1, ^h2

  block ^b1:
    %t1 = binary add i32 %s1, 2
    %j1 = binary add i32 %i1, 1
    jump ^h1

  block ^h2:
    %i2 = phi i32 [^h1: 0, ^b2: %j2]
    %s2 = phi i32 [^h1: %s1, ^b2: %t2]
    %c2 = cmp lt i32 %i2, 1000
    branch %c2, ^b2, ^h3

  block ^b2:
    %t2 = binary add i32 %s2, 3
    %j2 = binary add i32 %i2, 1
    jump ^h2

  block ^h3:
    %i3 = phi i32 [^h2: 0, ^b3: %j3]
    %s3 = phi i32 [^h2: %s2, ^b3: %t3]
    %c3 = cmp lt i32 %i3, 1000
    branch %c3, ^b3, ^h4

  block ^b3:
    %t3 = binary add i32 %s3, 4
    %j3 = binary add i32 %i3, 1
    jump ^h3

  block ^h4:
    %i4 = phi i32 [^h3: 0, ^b4: %j4]
    %s4 = phi i32 [^h3: %s3, ^b4: %t4]
    %c4 = cmp lt i32 %i4, 1000
    branch %c4, ^b4, ^h5

  block ^b4:
    %t4 = binary add i32 %s4, 5
    %j4 = binary add i32 %i4, 1
    jump ^h4

  block ^h5:
    %i5 = phi i32 [^h4: 0, ^b5: %j5]
    %s5 = phi i32 [^h4: %s4, ^b5: %t5]
    %c5 = cmp lt i32 %i5, 1000
    branch %c5, ^b5, ^h6

  block ^b5:
    %t5 = binary add i32 %s5, 6
    %j5 = binary add i32 %i5, 1
    jump ^h5

  block ^h6:
    %i6 = phi i32 [^h5: 0, ^b6: %j6]
    %s6 = phi i32 [^h5: %s5, ^b6: %t6]
    %c6 = cmp lt i32 %i6, 1000
    branch %c6, ^b6, ^h7

  block ^b6:
    %t6 = binary add i32 %s6, 7
    %j6 = binary add i32 %i6, 1
    jump ^h6

  block ^h7:
    %i7 = phi i32 [^h6: 0, ^b7: %j7]
    %s7 = phi i32 [^h6: %s6, ^b7: %t7]
    %c7 = cmp lt i32 %i7, 1000
    branch %c7, ^b7, ^h8

  block ^b7:
    %t7 = binary add i32 %s7, 8
    %j7 = binary add i32 %i7, 1
    jump ^h7

  block ^h8:
    %i8 = phi i32 [^h7: 0, ^b8: %j8]
    %s8 = phi i32 [^h7: %s7, ^b8: %t8]
    %c8 = cmp lt i32 %i8, 1000
    branch %c8, ^b8, ^h9

  block ^b8:
    %t8 = binary add i32 %s8, 9
    %j8 = binary add i32 %i8, 1
    jump ^h8

  block ^h9:
    %i9 = phi i32 [^h8: 0, ^b9: %j9]
    %s9 = phi i32 [^h8: %s8, ^b9: %t9]
    %c9 = cmp lt i32 %i9, 1000
    branch %c9, ^b9, ^h10

  block ^b9:
    %t9 = binary add i32 %s9, 10
    %j9 = binary add i32 %i9, 1
    jump ^h9

  block ^h10:
    %i10 = phi i32 [^h9: 0, ^b10: %j10]
    %s10 = phi i32 [^h9: %s9, ^b10: %t10]
    %c10 = cmp lt i32 %i10, 1000
    branch %c10, ^b10, ^h11

  block ^b10:
    %t10 = binary add i32 %s10, 11
    %j10 = binary add i32 %i10, 1
    jump ^h10

  block ^h11:
    %i11 = phi i32 [^h10: 0, ^b11: %j11]
    %s11 = phi i32 [^h10: %s10, ^b11: %t11]
    %c11 = cmp lt i32 %i11, 1000
    branch %c11, ^b11, ^done

  block ^b11:
    %t11 = binary add i32 %s11, 12
    %j11 = binary add i32 %i11, 1
    jump ^h11

  block ^done:
    %r = binary add i32 %s11, %n
    return i32 %r
}

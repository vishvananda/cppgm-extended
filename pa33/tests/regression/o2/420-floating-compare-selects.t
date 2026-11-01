# Every floating compare a select reads, on each order of its operands and on
# a NaN. One condition fuses into the cmov; eq and ne take two and do not.
# An x87 compare does not fuse either.
global @nan_bits : i64 = 9221120237041090560

function @lt_f80(%a : f64, %b : f64, %x : i64, %y : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %p = convert fpext f80 f64 %a
    %q = convert fpext f80 f64 %b
    %c = cmp lt f80 %p, %q
    %r = select i64 %c, %x, %y
    return i64 %r
}

function @ult_f80(%a : f64, %b : f64, %x : i64, %y : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %p = convert fpext f80 f64 %a
    %q = convert fpext f80 f64 %b
    %c = cmp ult f80 %p, %q
    %r = select i64 %c, %x, %y
    return i64 %r
}

function @lt_f64(%a : f64, %b : f64, %x : i64, %y : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %c = cmp lt f64 %a, %b
    %r = select i64 %c, %x, %y
    return i64 %r
}

function @le_f64(%a : f64, %b : f64, %x : i64, %y : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %c = cmp le f64 %a, %b
    %r = select i64 %c, %x, %y
    return i64 %r
}

function @gt_f64(%a : f64, %b : f64, %x : i64, %y : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %c = cmp gt f64 %a, %b
    %r = select i64 %c, %x, %y
    return i64 %r
}

function @ge_f64(%a : f64, %b : f64, %x : i64, %y : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %c = cmp ge f64 %a, %b
    %r = select i64 %c, %x, %y
    return i64 %r
}

function @ult_f64(%a : f64, %b : f64, %x : i64, %y : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %c = cmp ult f64 %a, %b
    %r = select i64 %c, %x, %y
    return i64 %r
}

function @ule_f64(%a : f64, %b : f64, %x : i64, %y : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %c = cmp ule f64 %a, %b
    %r = select i64 %c, %x, %y
    return i64 %r
}

function @ugt_f64(%a : f64, %b : f64, %x : i64, %y : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %c = cmp ugt f64 %a, %b
    %r = select i64 %c, %x, %y
    return i64 %r
}

function @uge_f64(%a : f64, %b : f64, %x : i64, %y : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %c = cmp uge f64 %a, %b
    %r = select i64 %c, %x, %y
    return i64 %r
}

function @eq_f64(%a : f64, %b : f64, %x : i64, %y : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %c = cmp eq f64 %a, %b
    %r = select i64 %c, %x, %y
    return i64 %r
}

function @ne_f64(%a : f64, %b : f64, %x : i64, %y : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %c = cmp ne f64 %a, %b
    %r = select i64 %c, %x, %y
    return i64 %r
}

function @lt_f32(%a : f32, %b : f32, %x : i64, %y : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %c = cmp lt f32 %a, %b
    %r = select i64 %c, %x, %y
    return i64 %r
}

function @le_f32(%a : f32, %b : f32, %x : i64, %y : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %c = cmp le f32 %a, %b
    %r = select i64 %c, %x, %y
    return i64 %r
}

function @gt_f32(%a : f32, %b : f32, %x : i64, %y : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %c = cmp gt f32 %a, %b
    %r = select i64 %c, %x, %y
    return i64 %r
}

function @ge_f32(%a : f32, %b : f32, %x : i64, %y : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %c = cmp ge f32 %a, %b
    %r = select i64 %c, %x, %y
    return i64 %r
}

function @ult_f32(%a : f32, %b : f32, %x : i64, %y : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %c = cmp ult f32 %a, %b
    %r = select i64 %c, %x, %y
    return i64 %r
}

function @ule_f32(%a : f32, %b : f32, %x : i64, %y : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %c = cmp ule f32 %a, %b
    %r = select i64 %c, %x, %y
    return i64 %r
}

function @ugt_f32(%a : f32, %b : f32, %x : i64, %y : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %c = cmp ugt f32 %a, %b
    %r = select i64 %c, %x, %y
    return i64 %r
}

function @uge_f32(%a : f32, %b : f32, %x : i64, %y : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %c = cmp uge f32 %a, %b
    %r = select i64 %c, %x, %y
    return i64 %r
}

function @eq_f32(%a : f32, %b : f32, %x : i64, %y : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %c = cmp eq f32 %a, %b
    %r = select i64 %c, %x, %y
    return i64 %r
}

function @ne_f32(%a : f32, %b : f32, %x : i64, %y : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %c = cmp ne f32 %a, %b
    %r = select i64 %c, %x, %y
    return i64 %r
}

# The select keeps the value it does not overwrite.
function @same_register(%a : f64, %b : f64, %x : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %c = cmp gt f64 %a, %b
    %r = select i64 %c, %x, 7
    return i64 %r
}

function @main() -> i64 [role=entry] {
  block ^entry:
    %cell = addr @nan_bits
    %nan = load f64 %cell
    %nan32 = convert fptrunc f32 f64 %nan
    jump ^b0

  block ^b0:
    %r0 = call i64 @lt_f64(1.0, 2.0, 11, 22)
    %ok0 = cmp eq i64 %r0, 11
    branch %ok0, ^b1, ^bad0

  block ^bad0:
    return i64 2

  block ^b1:
    %r1 = call i64 @lt_f64(2.0, 1.0, 11, 22)
    %ok1 = cmp eq i64 %r1, 22
    branch %ok1, ^b2, ^bad1

  block ^bad1:
    return i64 3

  block ^b2:
    %r2 = call i64 @lt_f64(1.0, 1.0, 11, 22)
    %ok2 = cmp eq i64 %r2, 22
    branch %ok2, ^b3, ^bad2

  block ^bad2:
    return i64 4

  block ^b3:
    %r3 = call i64 @lt_f64(%nan, 1.0, 11, 22)
    %ok3 = cmp eq i64 %r3, 22
    branch %ok3, ^b4, ^bad3

  block ^bad3:
    return i64 5

  block ^b4:
    %r4 = call i64 @le_f64(1.0, 2.0, 11, 22)
    %ok4 = cmp eq i64 %r4, 11
    branch %ok4, ^b5, ^bad4

  block ^bad4:
    return i64 6

  block ^b5:
    %r5 = call i64 @le_f64(2.0, 1.0, 11, 22)
    %ok5 = cmp eq i64 %r5, 22
    branch %ok5, ^b6, ^bad5

  block ^bad5:
    return i64 7

  block ^b6:
    %r6 = call i64 @le_f64(1.0, 1.0, 11, 22)
    %ok6 = cmp eq i64 %r6, 11
    branch %ok6, ^b7, ^bad6

  block ^bad6:
    return i64 8

  block ^b7:
    %r7 = call i64 @le_f64(%nan, 1.0, 11, 22)
    %ok7 = cmp eq i64 %r7, 22
    branch %ok7, ^b8, ^bad7

  block ^bad7:
    return i64 9

  block ^b8:
    %r8 = call i64 @gt_f64(1.0, 2.0, 11, 22)
    %ok8 = cmp eq i64 %r8, 22
    branch %ok8, ^b9, ^bad8

  block ^bad8:
    return i64 10

  block ^b9:
    %r9 = call i64 @gt_f64(2.0, 1.0, 11, 22)
    %ok9 = cmp eq i64 %r9, 11
    branch %ok9, ^b10, ^bad9

  block ^bad9:
    return i64 11

  block ^b10:
    %r10 = call i64 @gt_f64(1.0, 1.0, 11, 22)
    %ok10 = cmp eq i64 %r10, 22
    branch %ok10, ^b11, ^bad10

  block ^bad10:
    return i64 12

  block ^b11:
    %r11 = call i64 @gt_f64(%nan, 1.0, 11, 22)
    %ok11 = cmp eq i64 %r11, 22
    branch %ok11, ^b12, ^bad11

  block ^bad11:
    return i64 13

  block ^b12:
    %r12 = call i64 @ge_f64(1.0, 2.0, 11, 22)
    %ok12 = cmp eq i64 %r12, 22
    branch %ok12, ^b13, ^bad12

  block ^bad12:
    return i64 14

  block ^b13:
    %r13 = call i64 @ge_f64(2.0, 1.0, 11, 22)
    %ok13 = cmp eq i64 %r13, 11
    branch %ok13, ^b14, ^bad13

  block ^bad13:
    return i64 15

  block ^b14:
    %r14 = call i64 @ge_f64(1.0, 1.0, 11, 22)
    %ok14 = cmp eq i64 %r14, 11
    branch %ok14, ^b15, ^bad14

  block ^bad14:
    return i64 16

  block ^b15:
    %r15 = call i64 @ge_f64(%nan, 1.0, 11, 22)
    %ok15 = cmp eq i64 %r15, 22
    branch %ok15, ^b16, ^bad15

  block ^bad15:
    return i64 17

  block ^b16:
    %r16 = call i64 @ult_f64(1.0, 2.0, 11, 22)
    %ok16 = cmp eq i64 %r16, 11
    branch %ok16, ^b17, ^bad16

  block ^bad16:
    return i64 18

  block ^b17:
    %r17 = call i64 @ult_f64(2.0, 1.0, 11, 22)
    %ok17 = cmp eq i64 %r17, 22
    branch %ok17, ^b18, ^bad17

  block ^bad17:
    return i64 19

  block ^b18:
    %r18 = call i64 @ult_f64(1.0, 1.0, 11, 22)
    %ok18 = cmp eq i64 %r18, 22
    branch %ok18, ^b19, ^bad18

  block ^bad18:
    return i64 20

  block ^b19:
    %r19 = call i64 @ult_f64(%nan, 1.0, 11, 22)
    %ok19 = cmp eq i64 %r19, 11
    branch %ok19, ^b20, ^bad19

  block ^bad19:
    return i64 21

  block ^b20:
    %r20 = call i64 @ule_f64(1.0, 2.0, 11, 22)
    %ok20 = cmp eq i64 %r20, 11
    branch %ok20, ^b21, ^bad20

  block ^bad20:
    return i64 22

  block ^b21:
    %r21 = call i64 @ule_f64(2.0, 1.0, 11, 22)
    %ok21 = cmp eq i64 %r21, 22
    branch %ok21, ^b22, ^bad21

  block ^bad21:
    return i64 23

  block ^b22:
    %r22 = call i64 @ule_f64(1.0, 1.0, 11, 22)
    %ok22 = cmp eq i64 %r22, 11
    branch %ok22, ^b23, ^bad22

  block ^bad22:
    return i64 24

  block ^b23:
    %r23 = call i64 @ule_f64(%nan, 1.0, 11, 22)
    %ok23 = cmp eq i64 %r23, 11
    branch %ok23, ^b24, ^bad23

  block ^bad23:
    return i64 25

  block ^b24:
    %r24 = call i64 @ugt_f64(1.0, 2.0, 11, 22)
    %ok24 = cmp eq i64 %r24, 22
    branch %ok24, ^b25, ^bad24

  block ^bad24:
    return i64 26

  block ^b25:
    %r25 = call i64 @ugt_f64(2.0, 1.0, 11, 22)
    %ok25 = cmp eq i64 %r25, 11
    branch %ok25, ^b26, ^bad25

  block ^bad25:
    return i64 27

  block ^b26:
    %r26 = call i64 @ugt_f64(1.0, 1.0, 11, 22)
    %ok26 = cmp eq i64 %r26, 22
    branch %ok26, ^b27, ^bad26

  block ^bad26:
    return i64 28

  block ^b27:
    %r27 = call i64 @ugt_f64(%nan, 1.0, 11, 22)
    %ok27 = cmp eq i64 %r27, 11
    branch %ok27, ^b28, ^bad27

  block ^bad27:
    return i64 29

  block ^b28:
    %r28 = call i64 @uge_f64(1.0, 2.0, 11, 22)
    %ok28 = cmp eq i64 %r28, 22
    branch %ok28, ^b29, ^bad28

  block ^bad28:
    return i64 30

  block ^b29:
    %r29 = call i64 @uge_f64(2.0, 1.0, 11, 22)
    %ok29 = cmp eq i64 %r29, 11
    branch %ok29, ^b30, ^bad29

  block ^bad29:
    return i64 31

  block ^b30:
    %r30 = call i64 @uge_f64(1.0, 1.0, 11, 22)
    %ok30 = cmp eq i64 %r30, 11
    branch %ok30, ^b31, ^bad30

  block ^bad30:
    return i64 32

  block ^b31:
    %r31 = call i64 @uge_f64(%nan, 1.0, 11, 22)
    %ok31 = cmp eq i64 %r31, 11
    branch %ok31, ^b32, ^bad31

  block ^bad31:
    return i64 33

  block ^b32:
    %r32 = call i64 @eq_f64(1.0, 2.0, 11, 22)
    %ok32 = cmp eq i64 %r32, 22
    branch %ok32, ^b33, ^bad32

  block ^bad32:
    return i64 34

  block ^b33:
    %r33 = call i64 @eq_f64(2.0, 1.0, 11, 22)
    %ok33 = cmp eq i64 %r33, 22
    branch %ok33, ^b34, ^bad33

  block ^bad33:
    return i64 35

  block ^b34:
    %r34 = call i64 @eq_f64(1.0, 1.0, 11, 22)
    %ok34 = cmp eq i64 %r34, 11
    branch %ok34, ^b35, ^bad34

  block ^bad34:
    return i64 36

  block ^b35:
    %r35 = call i64 @eq_f64(%nan, 1.0, 11, 22)
    %ok35 = cmp eq i64 %r35, 22
    branch %ok35, ^b36, ^bad35

  block ^bad35:
    return i64 37

  block ^b36:
    %r36 = call i64 @ne_f64(1.0, 2.0, 11, 22)
    %ok36 = cmp eq i64 %r36, 11
    branch %ok36, ^b37, ^bad36

  block ^bad36:
    return i64 38

  block ^b37:
    %r37 = call i64 @ne_f64(2.0, 1.0, 11, 22)
    %ok37 = cmp eq i64 %r37, 11
    branch %ok37, ^b38, ^bad37

  block ^bad37:
    return i64 39

  block ^b38:
    %r38 = call i64 @ne_f64(1.0, 1.0, 11, 22)
    %ok38 = cmp eq i64 %r38, 22
    branch %ok38, ^b39, ^bad38

  block ^bad38:
    return i64 40

  block ^b39:
    %r39 = call i64 @ne_f64(%nan, 1.0, 11, 22)
    %ok39 = cmp eq i64 %r39, 11
    branch %ok39, ^b40, ^bad39

  block ^bad39:
    return i64 41

  block ^b40:
    %r40 = call i64 @lt_f32(1.0, 2.0, 11, 22)
    %ok40 = cmp eq i64 %r40, 11
    branch %ok40, ^b41, ^bad40

  block ^bad40:
    return i64 42

  block ^b41:
    %r41 = call i64 @lt_f32(2.0, 1.0, 11, 22)
    %ok41 = cmp eq i64 %r41, 22
    branch %ok41, ^b42, ^bad41

  block ^bad41:
    return i64 43

  block ^b42:
    %r42 = call i64 @lt_f32(1.0, 1.0, 11, 22)
    %ok42 = cmp eq i64 %r42, 22
    branch %ok42, ^b43, ^bad42

  block ^bad42:
    return i64 44

  block ^b43:
    %r43 = call i64 @lt_f32(%nan32, 1.0, 11, 22)
    %ok43 = cmp eq i64 %r43, 22
    branch %ok43, ^b44, ^bad43

  block ^bad43:
    return i64 45

  block ^b44:
    %r44 = call i64 @le_f32(1.0, 2.0, 11, 22)
    %ok44 = cmp eq i64 %r44, 11
    branch %ok44, ^b45, ^bad44

  block ^bad44:
    return i64 46

  block ^b45:
    %r45 = call i64 @le_f32(2.0, 1.0, 11, 22)
    %ok45 = cmp eq i64 %r45, 22
    branch %ok45, ^b46, ^bad45

  block ^bad45:
    return i64 47

  block ^b46:
    %r46 = call i64 @le_f32(1.0, 1.0, 11, 22)
    %ok46 = cmp eq i64 %r46, 11
    branch %ok46, ^b47, ^bad46

  block ^bad46:
    return i64 48

  block ^b47:
    %r47 = call i64 @le_f32(%nan32, 1.0, 11, 22)
    %ok47 = cmp eq i64 %r47, 22
    branch %ok47, ^b48, ^bad47

  block ^bad47:
    return i64 49

  block ^b48:
    %r48 = call i64 @gt_f32(1.0, 2.0, 11, 22)
    %ok48 = cmp eq i64 %r48, 22
    branch %ok48, ^b49, ^bad48

  block ^bad48:
    return i64 50

  block ^b49:
    %r49 = call i64 @gt_f32(2.0, 1.0, 11, 22)
    %ok49 = cmp eq i64 %r49, 11
    branch %ok49, ^b50, ^bad49

  block ^bad49:
    return i64 51

  block ^b50:
    %r50 = call i64 @gt_f32(1.0, 1.0, 11, 22)
    %ok50 = cmp eq i64 %r50, 22
    branch %ok50, ^b51, ^bad50

  block ^bad50:
    return i64 52

  block ^b51:
    %r51 = call i64 @gt_f32(%nan32, 1.0, 11, 22)
    %ok51 = cmp eq i64 %r51, 22
    branch %ok51, ^b52, ^bad51

  block ^bad51:
    return i64 53

  block ^b52:
    %r52 = call i64 @ge_f32(1.0, 2.0, 11, 22)
    %ok52 = cmp eq i64 %r52, 22
    branch %ok52, ^b53, ^bad52

  block ^bad52:
    return i64 54

  block ^b53:
    %r53 = call i64 @ge_f32(2.0, 1.0, 11, 22)
    %ok53 = cmp eq i64 %r53, 11
    branch %ok53, ^b54, ^bad53

  block ^bad53:
    return i64 55

  block ^b54:
    %r54 = call i64 @ge_f32(1.0, 1.0, 11, 22)
    %ok54 = cmp eq i64 %r54, 11
    branch %ok54, ^b55, ^bad54

  block ^bad54:
    return i64 56

  block ^b55:
    %r55 = call i64 @ge_f32(%nan32, 1.0, 11, 22)
    %ok55 = cmp eq i64 %r55, 22
    branch %ok55, ^b56, ^bad55

  block ^bad55:
    return i64 57

  block ^b56:
    %r56 = call i64 @ult_f32(1.0, 2.0, 11, 22)
    %ok56 = cmp eq i64 %r56, 11
    branch %ok56, ^b57, ^bad56

  block ^bad56:
    return i64 58

  block ^b57:
    %r57 = call i64 @ult_f32(2.0, 1.0, 11, 22)
    %ok57 = cmp eq i64 %r57, 22
    branch %ok57, ^b58, ^bad57

  block ^bad57:
    return i64 59

  block ^b58:
    %r58 = call i64 @ult_f32(1.0, 1.0, 11, 22)
    %ok58 = cmp eq i64 %r58, 22
    branch %ok58, ^b59, ^bad58

  block ^bad58:
    return i64 60

  block ^b59:
    %r59 = call i64 @ult_f32(%nan32, 1.0, 11, 22)
    %ok59 = cmp eq i64 %r59, 11
    branch %ok59, ^b60, ^bad59

  block ^bad59:
    return i64 61

  block ^b60:
    %r60 = call i64 @ule_f32(1.0, 2.0, 11, 22)
    %ok60 = cmp eq i64 %r60, 11
    branch %ok60, ^b61, ^bad60

  block ^bad60:
    return i64 62

  block ^b61:
    %r61 = call i64 @ule_f32(2.0, 1.0, 11, 22)
    %ok61 = cmp eq i64 %r61, 22
    branch %ok61, ^b62, ^bad61

  block ^bad61:
    return i64 63

  block ^b62:
    %r62 = call i64 @ule_f32(1.0, 1.0, 11, 22)
    %ok62 = cmp eq i64 %r62, 11
    branch %ok62, ^b63, ^bad62

  block ^bad62:
    return i64 64

  block ^b63:
    %r63 = call i64 @ule_f32(%nan32, 1.0, 11, 22)
    %ok63 = cmp eq i64 %r63, 11
    branch %ok63, ^b64, ^bad63

  block ^bad63:
    return i64 65

  block ^b64:
    %r64 = call i64 @ugt_f32(1.0, 2.0, 11, 22)
    %ok64 = cmp eq i64 %r64, 22
    branch %ok64, ^b65, ^bad64

  block ^bad64:
    return i64 66

  block ^b65:
    %r65 = call i64 @ugt_f32(2.0, 1.0, 11, 22)
    %ok65 = cmp eq i64 %r65, 11
    branch %ok65, ^b66, ^bad65

  block ^bad65:
    return i64 67

  block ^b66:
    %r66 = call i64 @ugt_f32(1.0, 1.0, 11, 22)
    %ok66 = cmp eq i64 %r66, 22
    branch %ok66, ^b67, ^bad66

  block ^bad66:
    return i64 68

  block ^b67:
    %r67 = call i64 @ugt_f32(%nan32, 1.0, 11, 22)
    %ok67 = cmp eq i64 %r67, 11
    branch %ok67, ^b68, ^bad67

  block ^bad67:
    return i64 69

  block ^b68:
    %r68 = call i64 @uge_f32(1.0, 2.0, 11, 22)
    %ok68 = cmp eq i64 %r68, 22
    branch %ok68, ^b69, ^bad68

  block ^bad68:
    return i64 70

  block ^b69:
    %r69 = call i64 @uge_f32(2.0, 1.0, 11, 22)
    %ok69 = cmp eq i64 %r69, 11
    branch %ok69, ^b70, ^bad69

  block ^bad69:
    return i64 71

  block ^b70:
    %r70 = call i64 @uge_f32(1.0, 1.0, 11, 22)
    %ok70 = cmp eq i64 %r70, 11
    branch %ok70, ^b71, ^bad70

  block ^bad70:
    return i64 72

  block ^b71:
    %r71 = call i64 @uge_f32(%nan32, 1.0, 11, 22)
    %ok71 = cmp eq i64 %r71, 11
    branch %ok71, ^b72, ^bad71

  block ^bad71:
    return i64 73

  block ^b72:
    %r72 = call i64 @eq_f32(1.0, 2.0, 11, 22)
    %ok72 = cmp eq i64 %r72, 22
    branch %ok72, ^b73, ^bad72

  block ^bad72:
    return i64 74

  block ^b73:
    %r73 = call i64 @eq_f32(2.0, 1.0, 11, 22)
    %ok73 = cmp eq i64 %r73, 22
    branch %ok73, ^b74, ^bad73

  block ^bad73:
    return i64 75

  block ^b74:
    %r74 = call i64 @eq_f32(1.0, 1.0, 11, 22)
    %ok74 = cmp eq i64 %r74, 11
    branch %ok74, ^b75, ^bad74

  block ^bad74:
    return i64 76

  block ^b75:
    %r75 = call i64 @eq_f32(%nan32, 1.0, 11, 22)
    %ok75 = cmp eq i64 %r75, 22
    branch %ok75, ^b76, ^bad75

  block ^bad75:
    return i64 77

  block ^b76:
    %r76 = call i64 @ne_f32(1.0, 2.0, 11, 22)
    %ok76 = cmp eq i64 %r76, 11
    branch %ok76, ^b77, ^bad76

  block ^bad76:
    return i64 78

  block ^b77:
    %r77 = call i64 @ne_f32(2.0, 1.0, 11, 22)
    %ok77 = cmp eq i64 %r77, 11
    branch %ok77, ^b78, ^bad77

  block ^bad77:
    return i64 79

  block ^b78:
    %r78 = call i64 @ne_f32(1.0, 1.0, 11, 22)
    %ok78 = cmp eq i64 %r78, 22
    branch %ok78, ^b79, ^bad78

  block ^bad78:
    return i64 80

  block ^b79:
    %r79 = call i64 @ne_f32(%nan32, 1.0, 11, 22)
    %ok79 = cmp eq i64 %r79, 11
    branch %ok79, ^b80, ^bad79

  block ^bad79:
    return i64 81

  block ^b80:
    %s1 = call i64 @same_register(2.0, 1.0, 40)
    %s2 = call i64 @same_register(1.0, 2.0, 40)
    %s3 = call i64 @same_register(%nan, 1.0, 40)
    %sum = binary add i64 %s1, %s2
    %sum2 = binary add i64 %sum, %s3
    %ok = cmp eq i64 %sum2, 54
    branch %ok, ^pass, ^fail

  block ^fail:
    return i64 121

  block ^pass:
    %f0 = call i64 @lt_f80(1.0, 2.0, 11, 22)
    %f1 = call i64 @lt_f80(2.0, 1.0, 11, 22)
    %f2 = call i64 @lt_f80(%nan, 1.0, 11, 22)
    %f3 = call i64 @ult_f80(%nan, 1.0, 11, 22)
    %f4 = binary add i64 %f0, %f1
    %f5 = binary add i64 %f2, %f3
    %f6 = binary add i64 %f4, %f5
    %fok = cmp eq i64 %f6, 66
    branch %fok, ^done, ^fail80

  block ^fail80:
    return i64 122

  block ^done:
    return i64 0
}

# A truth test of a floating compare inverts it. The negation of an ordered
# predicate is its unordered form: it holds on a NaN.
function @not_lt_f64(%a : f64, %b : f64) -> i64 {
  block ^entry:
    %c = cmp lt f64 %a, %b
    %t = cmp eq i64 %c, 0
    branch %t, ^yes, ^no

  block ^yes:
    return i64 1

  block ^no:
    return i64 2
}

function @not_le_f64(%a : f64, %b : f64) -> i64 {
  block ^entry:
    %c = cmp le f64 %a, %b
    %t = cmp eq i64 %c, 0
    branch %t, ^yes, ^no

  block ^yes:
    return i64 1

  block ^no:
    return i64 2
}

function @not_gt_f64(%a : f64, %b : f64) -> i64 {
  block ^entry:
    %c = cmp gt f64 %a, %b
    %t = cmp eq i64 %c, 0
    branch %t, ^yes, ^no

  block ^yes:
    return i64 1

  block ^no:
    return i64 2
}

function @not_ge_f64(%a : f64, %b : f64) -> i64 {
  block ^entry:
    %c = cmp ge f64 %a, %b
    %t = cmp eq i64 %c, 0
    branch %t, ^yes, ^no

  block ^yes:
    return i64 1

  block ^no:
    return i64 2
}

function @not_ult_f64(%a : f64, %b : f64) -> i64 {
  block ^entry:
    %c = cmp ult f64 %a, %b
    %t = cmp eq i64 %c, 0
    branch %t, ^yes, ^no

  block ^yes:
    return i64 1

  block ^no:
    return i64 2
}

function @not_ule_f64(%a : f64, %b : f64) -> i64 {
  block ^entry:
    %c = cmp ule f64 %a, %b
    %t = cmp eq i64 %c, 0
    branch %t, ^yes, ^no

  block ^yes:
    return i64 1

  block ^no:
    return i64 2
}

function @not_ugt_f64(%a : f64, %b : f64) -> i64 {
  block ^entry:
    %c = cmp ugt f64 %a, %b
    %t = cmp eq i64 %c, 0
    branch %t, ^yes, ^no

  block ^yes:
    return i64 1

  block ^no:
    return i64 2
}

function @not_uge_f64(%a : f64, %b : f64) -> i64 {
  block ^entry:
    %c = cmp uge f64 %a, %b
    %t = cmp eq i64 %c, 0
    branch %t, ^yes, ^no

  block ^yes:
    return i64 1

  block ^no:
    return i64 2
}

function @not_eq_f64(%a : f64, %b : f64) -> i64 {
  block ^entry:
    %c = cmp eq f64 %a, %b
    %t = cmp eq i64 %c, 0
    branch %t, ^yes, ^no

  block ^yes:
    return i64 1

  block ^no:
    return i64 2
}

function @not_ne_f64(%a : f64, %b : f64) -> i64 {
  block ^entry:
    %c = cmp ne f64 %a, %b
    %t = cmp eq i64 %c, 0
    branch %t, ^yes, ^no

  block ^yes:
    return i64 1

  block ^no:
    return i64 2
}

function @not_lt_f32(%a : f32, %b : f32) -> i64 {
  block ^entry:
    %c = cmp lt f32 %a, %b
    %t = cmp eq i64 %c, 0
    branch %t, ^yes, ^no

  block ^yes:
    return i64 1

  block ^no:
    return i64 2
}

function @not_le_f32(%a : f32, %b : f32) -> i64 {
  block ^entry:
    %c = cmp le f32 %a, %b
    %t = cmp eq i64 %c, 0
    branch %t, ^yes, ^no

  block ^yes:
    return i64 1

  block ^no:
    return i64 2
}

function @not_gt_f32(%a : f32, %b : f32) -> i64 {
  block ^entry:
    %c = cmp gt f32 %a, %b
    %t = cmp eq i64 %c, 0
    branch %t, ^yes, ^no

  block ^yes:
    return i64 1

  block ^no:
    return i64 2
}

function @not_ge_f32(%a : f32, %b : f32) -> i64 {
  block ^entry:
    %c = cmp ge f32 %a, %b
    %t = cmp eq i64 %c, 0
    branch %t, ^yes, ^no

  block ^yes:
    return i64 1

  block ^no:
    return i64 2
}

function @not_ult_f32(%a : f32, %b : f32) -> i64 {
  block ^entry:
    %c = cmp ult f32 %a, %b
    %t = cmp eq i64 %c, 0
    branch %t, ^yes, ^no

  block ^yes:
    return i64 1

  block ^no:
    return i64 2
}

function @not_ule_f32(%a : f32, %b : f32) -> i64 {
  block ^entry:
    %c = cmp ule f32 %a, %b
    %t = cmp eq i64 %c, 0
    branch %t, ^yes, ^no

  block ^yes:
    return i64 1

  block ^no:
    return i64 2
}

function @not_ugt_f32(%a : f32, %b : f32) -> i64 {
  block ^entry:
    %c = cmp ugt f32 %a, %b
    %t = cmp eq i64 %c, 0
    branch %t, ^yes, ^no

  block ^yes:
    return i64 1

  block ^no:
    return i64 2
}

function @not_uge_f32(%a : f32, %b : f32) -> i64 {
  block ^entry:
    %c = cmp uge f32 %a, %b
    %t = cmp eq i64 %c, 0
    branch %t, ^yes, ^no

  block ^yes:
    return i64 1

  block ^no:
    return i64 2
}

function @not_eq_f32(%a : f32, %b : f32) -> i64 {
  block ^entry:
    %c = cmp eq f32 %a, %b
    %t = cmp eq i64 %c, 0
    branch %t, ^yes, ^no

  block ^yes:
    return i64 1

  block ^no:
    return i64 2
}

function @not_ne_f32(%a : f32, %b : f32) -> i64 {
  block ^entry:
    %c = cmp ne f32 %a, %b
    %t = cmp eq i64 %c, 0
    branch %t, ^yes, ^no

  block ^yes:
    return i64 1

  block ^no:
    return i64 2
}

function @not_one(%a : f64, %b : f64) -> i64 {
  block ^entry:
    %c = cmp lt f64 %a, %b
    %t = cmp ne i64 %c, 1
    %r = select i64 %t, 3, 4
    return i64 %r
}

function @through_truncation(%a : f80, %b : f80) -> i64 {
  block ^entry:
    %c = cmp gt f80 %a, %b
    %n = convert trunc u8 i64 %c
    %t = cmp eq u8 %n, 0
    %r = select i64 %t, 5, 6
    return i64 %r
}

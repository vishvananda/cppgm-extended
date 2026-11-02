# A floating choice between the two values its compare reads is a lane `min` or
# `max`, which give the second operand when unordered.

function @min_lt(%a : f64, %b : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %c = cmp lt f64 %a, %b
    branch %c, ^t, ^f
  block ^t:
    jump ^out
  block ^f:
    jump ^out
  block ^out:
    %r = phi f64 [^t: %a, ^f: %b]
    return f64 %r
}

function @max_gt(%a : f32, %b : f32) -> f32 [no_inline=yes] {
  block ^entry:
    %c = cmp gt f32 %a, %b
    branch %c, ^t, ^f
  block ^t:
    jump ^out
  block ^f:
    jump ^out
  block ^out:
    %r = phi f32 [^t: %a, ^f: %b]
    return f32 %r
}

# The opposite values swap the operands: an unordered pair keeps the same one.
function @max_of_lt(%a : f64, %b : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %c = cmp lt f64 %a, %b
    branch %c, ^t, ^f
  block ^t:
    jump ^out
  block ^f:
    jump ^out
  block ^out:
    %r = phi f64 [^t: %b, ^f: %a]
    return f64 %r
}

function @min_of_gt(%a : f64, %b : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %c = cmp gt f64 %a, %b
    branch %c, ^t, ^f
  block ^t:
    jump ^out
  block ^f:
    jump ^out
  block ^out:
    %r = phi f64 [^t: %b, ^f: %a]
    return f64 %r
}

# `<=` against a literal other than zero: equal values have one spelling.
function @min_le_literal(%a : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %c = cmp le f64 %a, 2.5
    branch %c, ^t, ^f
  block ^t:
    jump ^out
  block ^f:
    jump ^out
  block ^out:
    %r = phi f64 [^t: %a, ^f: 2.5]
    return f64 %r
}

function @min_ge_literal(%a : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %c = cmp ge f64 %a, 2.5
    branch %c, ^t, ^f
  block ^t:
    jump ^out
  block ^f:
    jump ^out
  block ^out:
    %r = phi f64 [^t: 2.5, ^f: %a]
    return f64 %r
}

# `<=` can tell +0 from -0 where `min` cannot.
function @le_values(%a : f64, %b : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %c = cmp le f64 %a, %b
    branch %c, ^t, ^f
  block ^t:
    jump ^out
  block ^f:
    jump ^out
  block ^out:
    %r = phi f64 [^t: %a, ^f: %b]
    return f64 %r
}

function @le_zero(%a : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %c = cmp le f64 %a, 0
    branch %c, ^t, ^f
  block ^t:
    jump ^out
  block ^f:
    jump ^out
  block ^out:
    %r = phi f64 [^t: %a, ^f: 0]
    return f64 %r
}

# Values other than the compare's stay a branch.
function @other(%a : f64, %b : f64, %d : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %c = cmp lt f64 %a, %b
    branch %c, ^t, ^f
  block ^t:
    jump ^out
  block ^f:
    jump ^out
  block ^out:
    %r = phi f64 [^t: %a, ^f: %d]
    return f64 %r
}

# Calls of the C library's math functions become instructions: fabs clears the
# sign bit, abs selects the negation, pow of a literal 2, 1, 0 or -1 is simple
# arithmetic, and sqrt of a value that cannot be negative is `unary sqrt`.

declare function @fabs(%x : f64) -> f64 [linkage=c, binding=strong, object=fabs]
declare function @fabsf(%x : f32) -> f32 [linkage=c, binding=strong, object=fabsf]
declare function @abs(%x : i32) -> i32 [linkage=c, binding=strong, object=abs]
declare function @labs(%x : i64) -> i64 [linkage=c, binding=strong, object=labs]
declare function @llabs(%x : i64) -> i64 [linkage=c, binding=strong, object=llabs]
declare function @pow(%x : f64, %y : f64) -> f64 [linkage=c, binding=strong, object=pow]
declare function @powf(%x : f32, %y : f32) -> f32 [linkage=c, binding=strong, object=powf]
declare function @sqrt(%x : f64) -> f64 [linkage=c, binding=strong, object=sqrt]
declare function @sqrtf(%x : f32) -> f32 [linkage=c, binding=strong, object=sqrtf]
# A library function outside the list, and one of the names without C linkage.
declare function @cbrt(%x : f64) -> f64 [linkage=c, binding=strong, object=cbrt]
declare function @pow_cxx(%x : f64, %y : f64) -> f64 [binding=strong, object=pow]

function @of_fabs(%x : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %r = call f64 @fabs(%x)
    return f64 %r
}

function @of_fabsf(%x : f32) -> f32 [no_inline=yes] {
  block ^entry:
    %r = call f32 @fabsf(%x)
    return f32 %r
}

function @of_abs(%x : i32) -> i32 [no_inline=yes] {
  block ^entry:
    %r = call i32 @abs(%x)
    return i32 %r
}

function @of_labs(%x : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %r = call i64 @labs(%x)
    return i64 %r
}

function @of_llabs(%x : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %r = call i64 @llabs(%x)
    return i64 %r
}

function @pow_two(%x : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %r = call f64 @pow(%x, 2.0)
    return f64 %r
}

function @pow_one(%x : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %r = call f64 @pow(%x, 1.0)
    return f64 %r
}

function @pow_zero(%x : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %r = call f64 @pow(%x, 0.0)
    return f64 %r
}

function @pow_inverse(%x : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %r = call f64 @pow(%x, -1.0)
    return f64 %r
}

function @pow_half(%x : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %r = call f64 @pow(%x, 0.5)
    return f64 %r
}

function @pow_three(%x : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %r = call f64 @pow(%x, 3.0)
    return f64 %r
}

function @pow_any(%x : f64, %y : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %r = call f64 @pow(%x, %y)
    return f64 %r
}

function @powf_two(%x : f32) -> f32 [no_inline=yes] {
  block ^entry:
    %r = call f32 @powf(%x, 2.0f)
    return f32 %r
}

function @powf_inverse(%x : f32) -> f32 [no_inline=yes] {
  block ^entry:
    %r = call f32 @powf(%x, -1.0f)
    return f32 %r
}

function @pow_of_float(%x : f32) -> f32 [no_inline=yes] {
  block ^entry:
    %r = call f32 @powf(%x, 0.5f)
    return f32 %r
}

function @other_pow(%x : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %r = call f64 @pow_cxx(%x, 2.0)
    return f64 %r
}

function @other_name(%x : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %r = call f64 @cbrt(%x)
    return f64 %r
}

function @sqrt_of_square(%x : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %m = binary mul f64 %x, %x
    %r = call f64 @sqrt(%m)
    return f64 %r
}

function @sqrt_of_sum(%x : f64, %y : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %a = binary mul f64 %x, %x
    %b = binary mul f64 %y, %y
    %s = binary add f64 %a, %b
    %r = call f64 @sqrt(%s)
    return f64 %r
}

function @sqrt_of_literal_sum(%x : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %a = binary mul f64 %x, %x
    %s = binary add f64 %a, 1.0
    %r = call f64 @sqrt(%s)
    return f64 %r
}

function @sqrt_of_ratio(%x : f64, %y : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %a = binary mul f64 %x, %x
    %b = binary mul f64 %y, %y
    %q = binary div f64 %a, %b
    %r = call f64 @sqrt(%q)
    return f64 %r
}

function @sqrt_of_product(%x : f64, %y : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %a = binary mul f64 %x, %x
    %b = binary mul f64 %y, %y
    %q = binary mul f64 %a, %b
    %r = call f64 @sqrt(%q)
    return f64 %r
}

function @sqrt_of_sqrt(%x : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %a = binary mul f64 %x, %x
    %b = call f64 @sqrt(%a)
    %r = call f64 @sqrt(%b)
    return f64 %r
}

function @sqrtf_of_square(%x : f32) -> f32 [no_inline=yes] {
  block ^entry:
    %m = binary mul f32 %x, %x
    %r = call f32 @sqrtf(%m)
    return f32 %r
}

function @sqrt_of_any(%x : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %r = call f64 @sqrt(%x)
    return f64 %r
}

function @sqrt_of_difference(%x : f64, %y : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %a = binary mul f64 %x, %x
    %s = binary sub f64 %a, %y
    %r = call f64 @sqrt(%s)
    return f64 %r
}

function @sqrt_of_mixed(%x : f64, %y : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %a = binary mul f64 %x, %x
    %s = binary mul f64 %a, %y
    %r = call f64 @sqrt(%s)
    return f64 %r
}

function @pow_of_literal() -> f64 [no_inline=yes] {
  block ^entry:
    %r = call f64 @pow(1.5, 2.0)
    return f64 %r
}

function @sqrt_of_negative_literal(%x : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %a = binary mul f64 %x, %x
    %s = binary add f64 %a, -1.0
    %r = call f64 @sqrt(%s)
    return f64 %r
}

function @sqrt_of_squares_difference(%x : f64, %y : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %a = binary mul f64 %x, %x
    %b = binary mul f64 %y, %y
    %s = binary sub f64 %a, %b
    %r = call f64 @sqrt(%s)
    return f64 %r
}

function @sqrt_of_negated(%x : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %a = binary mul f64 %x, %x
    %s = unary neg f64 %a
    %r = call f64 @sqrt(%s)
    return f64 %r
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}

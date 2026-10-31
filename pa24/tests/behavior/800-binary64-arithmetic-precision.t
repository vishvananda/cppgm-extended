# Each binary64 operation rounds once in its declared format. The long
# double check also keeps the separate extended-precision path executable.
function @arithmetic(%one : f64, %small : f64, %negative_small : f64,
                     %a : f64, %b : f64, %c : f64, %d : f64) -> i64 {
  block ^entry:
    %sum = binary add f64 %one, %small
    %difference = binary sub f64 %one, %negative_small
    %product = binary mul f64 %a, %b
    %quotient = binary div f64 %c, %d
    %bad_sum = cmp ne f64 %sum, 1.0000000000000002
    %bad_difference = cmp ne f64 %difference, 1.0000000000000002
    %bad_product = cmp ne f64 %product, 1.3618711226611643e121
    %bad_quotient = cmp ne f64 %quotient, 4.9200081039929301e-226
    %bad_additive = binary or i64 %bad_sum, %bad_difference
    %bad_multiplicative = binary or i64 %bad_product, %bad_quotient
    %bad = binary or i64 %bad_additive, %bad_multiplicative
    return i64 %bad
}
function @extended(%one : f80, %small : f80) -> i64 {
  block ^entry:
    %sum = binary add f80 %one, %small
    %difference = binary sub f80 %sum, %one
    %bad = cmp ne f80 %difference, %small
    return i64 %bad
}
function @main() -> i64 [role=entry] {
  block ^entry:
    %bad_double = call i64 @arithmetic(1.0, 1.1104940751682779e-16, -1.1104940751682779e-16, -1.3290626730907361e20, -1.0246854044092082e101, 1.3608547793652755e-308, 2.765960442749773e-83)
    %bad_extended = call i64 @extended(1.0L, 8.67361737988403547205962240695953369140625e-19L)
    %bad = binary or i64 %bad_double, %bad_extended
    return i64 %bad
}

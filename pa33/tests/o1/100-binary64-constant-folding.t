# Folding must use the same binary64 precision as an executed operation,
# including the cases where extended intermediates would double-round.
function @main() -> i64 [role=entry] {
  block ^entry:
    %sum = binary add f64 1.0, 1.1104940751682779e-16
    %difference = binary sub f64 1.0, -1.1104940751682779e-16
    %product = binary mul f64 -1.3290626730907361e20, -1.0246854044092082e101
    %quotient = binary div f64 1.3608547793652755e-308, 2.765960442749773e-83
    %bad_sum = cmp ne f64 %sum, 1.0000000000000002
    %bad_difference = cmp ne f64 %difference, 1.0000000000000002
    %bad_product = cmp ne f64 %product, 1.3618711226611643e121
    %bad_quotient = cmp ne f64 %quotient, 4.9200081039929301e-226
    %bad_additive = binary or i64 %bad_sum, %bad_difference
    %bad_multiplicative = binary or i64 %bad_product, %bad_quotient
    %bad = binary or i64 %bad_additive, %bad_multiplicative
    return i64 %bad
}

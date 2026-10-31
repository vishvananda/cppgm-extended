# Copy preserves the representation of live floating arithmetic results.
# The source must stay in XMM for this to test register-class transfers.
function @bits32(%x : f32) -> i32 {
  block ^entry:
    %twice = binary mul f32 %x, 2.0f
    %bits = copy i32 %twice
    return i32 %bits
}
function @bits64(%x : f64) -> i64 {
  block ^entry:
    %twice = binary mul f64 %x, 2.0
    %bits = copy i64 %twice
    return i64 %bits
}
function @roundtrip(%x : i64) -> i64 {
  block ^entry:
    %value = copy f64 %x
    %negative = unary neg f64 %value
    %bits = copy i64 %negative
    return i64 %bits
}
function @main() -> i64 [role=entry] {
  block ^entry:
    %single = call i32 @bits32(-0.5f)
    %bad_single = cmp ne i32 %single, -1082130432
    %single_zero = call i32 @bits32(-0.0f)
    %bad_single_zero = cmp ne i32 %single_zero, -2147483648
    %double = call i64 @bits64(1.0)
    %bad_double = cmp ne i64 %double, 4611686018427387904
    %double_zero = call i64 @bits64(-0.0)
    %bad_double_zero = cmp ne i64 %double_zero, -9223372036854775808
    %round = call i64 @roundtrip(4607182418800017408)
    %bad_round = cmp ne i64 %round, -4616189618054758400
    %bad0 = binary or i64 %bad_single, %bad_single_zero
    %bad1 = binary or i64 %bad_double, %bad_double_zero
    %bad2 = binary or i64 %bad0, %bad1
    %bad = binary or i64 %bad2, %bad_round
    return i64 %bad
}

global @routes [storage=readonly] = {
  ptr addr @normal_route
  ptr addr @fallback_route
}

# The ordinary join can use the values stored by its two incoming paths.
# The other write on each arm must retain its selected value and order.
function @normal_join(%which : i64, %flag : i1, %output : ptr) -> i32 [no_inline=yes] {
  slot $value : i32
  code_label @normal_route = ^route
  code_label @fallback_route = ^fallback
  block ^entry:
    store i32 43, $value
    %at = index ptr @routes, %which
    %target = load ptr %at
    indirect_jump %target, ^route, ^fallback
  block ^route:
    %old = load i32 $value
    branch %flag, ^yes, ^no
  block ^yes:
    %yes_value = binary add i32 %old, 37
    store i32 %yes_value, $value
    store i32 11, %output
    jump ^join
  block ^no:
    %no_value = binary add i32 %old, 41
    store i32 %no_value, $value
    store i32 13, %output
    jump ^join
  block ^join:
    %result = load i32 $value
    return i32 %result
  block ^fallback:
    store i32 59, $value
    %again = addr @normal_route
    indirect_jump %again, ^route
}

# A persistent code address can enter this join. It must retain its entry
# state, and it cannot acquire phi inputs that an indirect edge cannot pass.
function @addressed_join(%flag : i1) -> i32 [no_inline=yes] {
  slot $value : i32
  code_label @shared_entry = ^join
  block ^entry:
    branch %flag, ^yes, ^no
  block ^yes:
    store i32 19, $value
    %target = addr @shared_entry
    indirect_jump %target, ^join
  block ^no:
    store i32 23, $value
    jump ^join
  block ^join:
    %result = load i32 $value
    return i32 %result
}

function @main() -> i32 {
  slot $output : i32
  block ^entry:
    %out = addr $output
    %a = call i32 @normal_join(0, 0, %out)
    %out_a = load i32 $output
    %b = call i32 @normal_join(0, 1, %out)
    %out_b = load i32 $output
    %c = call i32 @normal_join(1, 0, %out)
    %out_c = load i32 $output
    %d = call i32 @normal_join(1, 1, %out)
    %out_d = load i32 $output
    %e = call i32 @addressed_join(0)
    %f = call i32 @addressed_join(1)
    %bad_a = cmp ne i32 %a, 84
    %bad_b = cmp ne i32 %b, 80
    %bad_c = cmp ne i32 %c, 100
    %bad_d = cmp ne i32 %d, 96
    %bad_e = cmp ne i32 %e, 23
    %bad_f = cmp ne i32 %f, 19
    %bad_out_a = cmp ne i32 %out_a, 13
    %bad_out_b = cmp ne i32 %out_b, 11
    %bad_out_c = cmp ne i32 %out_c, 13
    %bad_out_d = cmp ne i32 %out_d, 11
    %ab = binary or i64 %bad_a, %bad_b
    %cd = binary or i64 %bad_c, %bad_d
    %ef = binary or i64 %bad_e, %bad_f
    %out_ab = binary or i64 %bad_out_a, %bad_out_b
    %out_cd = binary or i64 %bad_out_c, %bad_out_d
    %abcd = binary or i64 %ab, %cd
    %values = binary or i64 %abcd, %ef
    %outs = binary or i64 %out_ab, %out_cd
    %bad = binary or i64 %values, %outs
    %status = convert trunc i32 i64 %bad
    return i32 %status
}

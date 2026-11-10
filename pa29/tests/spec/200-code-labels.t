global @dispatch_table = {
  ptr addr @first_address
  ptr addr @second_address
}
function @shared_dispatch(%which : i64, %n : i32) -> i32 {
  code_label @first_address = ^first
  code_label @second_address = ^second
  block ^entry:
    %entry = index ptr @dispatch_table, %which
    %target = load ptr %entry
    indirect_jump %target, ^first, ^second
  block ^first:
    %a = binary add i32 %n, 13
    return i32 %a
  block ^second:
    %b = binary mul i32 %n, 7
    return i32 %b
}

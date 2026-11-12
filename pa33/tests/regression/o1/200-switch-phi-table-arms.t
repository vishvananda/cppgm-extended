# Constant cases of a switch that meet in a phi become a table load; the
# arms the table replaces end unreachable before the cleanup drops them.
function @lookup(%n : i64, %seed : i64) -> i64 [no_inline=yes] {
  block ^entry:
    switch %n, ^otherwise, 0:^case0, 1:^case1, 2:^case2, 3:^case3, 4:^case4

  block ^case0:
    jump ^merge

  block ^case1:
    jump ^merge

  block ^case2:
    jump ^merge

  block ^case3:
    jump ^merge

  block ^case4:
    jump ^merge

  block ^otherwise:
    %fallback = binary add i64 %seed, 1
    jump ^merge

  block ^merge:
    %result = phi i64 [^case0: 7, ^case1: 3, ^case2: 9, ^case3: 4, ^case4: 6, ^otherwise: %fallback]
    return i64 %result
}

function @main() -> i32 {
  block ^entry:
    %zero = call i64 @lookup(0, 40)
    %four = call i64 @lookup(4, 40)
    %other = call i64 @lookup(5, 40)
    %zero_ok = cmp eq i64 %zero, 7
    %four_ok = cmp eq i64 %four, 6
    %other_ok = cmp eq i64 %other, 41
    %zero_bit = select i32 %zero_ok, 0, 1
    %four_bit = select i32 %four_ok, 0, 2
    %other_bit = select i32 %other_ok, 0, 4
    %some = binary or i32 %zero_bit, %four_bit
    %status = binary or i32 %some, %other_bit
    return i32 %status
}

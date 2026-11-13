# Store motion keeps $sizes[$channel], which the innermost loop (^at_loop)
# adds to on every pass, in a register: it loads the element before that loop
# and stores it after the loop. Both new instructions are in the loop around
# it (^pass_loop), which reads the same element. That loop waits for the next
# round, because this round's block map has no entries for the new
# instructions. (A bounds-checked build of the optimizer read past the map on
# this input. The unused load in ^channel_loop and the unreachable ^spin keep
# the instruction numbering that showed it.)
function @estimate(%data : ptr, %count : i64, %size : i64, %k : i64, %block : i64, %skip : i64, %channels : i32, %rot : i32) -> i32 [binding=strong] {
  slot $count : i64
  slot $channels : i32
  slot $sizes : obj<24x8>
  slot $start : i64
  slot $limit : i64
  slot $cond1 : i64
  slot $channel : i32
  slot $pass : i64
  slot $at : i64
  slot $size : i64
  slot $best : i64
  slot $cond2 : i64

  block ^entry:
    store typed i64 %count, $count
    jump ^limit

  block ^limit:
    %t13 = load typed i64 $count
    %t14 = load typed i64 $start
    %t15 = binary sub i64 %t13, %t14
    store typed i64 %t15, $cond1
    jump ^limit_end

  block ^limit_end:
    %t16 = load typed i64 $cond1
    store typed i64 %t16, $limit
    jump ^channel_loop

  block ^channel_loop:
    %t53 = load typed i32 $channels
    jump ^pass_loop

  block ^pass_loop:
    %t56 = load typed i64 $pass
    %t57 = convert sext i64 i32 4
    %t58 = cmp ult i64 %t56, %t57
    branch %t58, ^at_loop, ^channel_next

  block ^at_loop:
    %t77 = load typed i64 $at
    %t78 = load typed i64 $limit
    %t79 = cmp ult i64 %t77, %t78
    branch %t79, ^at_body, ^pass_next

  block ^at_body:
    %t89 = copy i64 16
    store typed i64 %t89, $size
    jump ^choose

  block ^choose:
    %t104 = load typed i64 $size
    store typed i64 %t104, $cond2
    jump ^chosen

  block ^chosen:
    %t105 = load typed i64 $cond2
    store typed i64 %t105, $best
    jump ^add

  block ^add:
    %t118 = load typed i64 $best
    %t119 = addr $sizes
    %t120 = load typed i32 $channel
    %t121 = index inbounds i64 [projection=array_element] %t119, %t120
    %t122 = load typed i64 %t121
    %t123 = binary add i64 %t122, %t118
    store typed i64 %t123, %t121
    jump ^at_next

  block ^at_next:
    %t124 = load typed i64 $at
    %t125 = binary add i64 %t124, 16
    store typed i64 %t125, $at
    jump ^at_loop

  block ^pass_next:
    %t126 = load typed i64 $pass
    %t127 = binary add i64 %t126, 1
    store typed i64 %t127, $pass
    jump ^pass_loop

  block ^channel_next:
    %t128 = load typed i32 $channel
    %t129 = binary add nowrap i32 %t128, 1
    store typed i32 %t129, $channel
    jump ^channel_loop

  block ^spin:
    jump ^spin
}

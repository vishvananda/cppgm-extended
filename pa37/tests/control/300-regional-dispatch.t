# A small irreducible cycle must not dispatch the surrounding hot loop.
target wasm32 1

function @regional_hot(%choose : i32, %iterations : i32) -> i32 {
  slot $value : i32
  block ^entry:
    branch %choose, ^left, ^right
  block ^left:
    %lv = phi i32 [^entry: 0, ^right: %rn]
    %ln = binary add i32 %lv, 1
    %ldone = cmp ge i32 %ln, 3
    branch %ldone, ^hot_entry, ^right
  block ^right:
    %rv = phi i32 [^entry: 0, ^left: %ln]
    %rn = binary add i32 %rv, 1
    %rdone = cmp ge i32 %rn, 3
    branch %rdone, ^hot_entry, ^left
  block ^hot_entry:
    %seed = phi i32 [^left: %ln, ^right: %rn]
    store volatile typed i32 %seed, $value
    %positive = cmp gt i32 %iterations, 0
    branch %positive, ^hot0, ^done
  block ^hot0:
    %n = phi i32 [^hot_entry: 0, ^hot11: %next]
    %v0 = load volatile typed i32 $value
    %a0 = binary add i32 %v0, 1
    store volatile typed i32 %a0, $value
    %stop0 = cmp eq i32 %a0, -1
    branch %stop0, ^done, ^hot1
  block ^hot1:
    %v1 = load volatile typed i32 $value
    %a1 = binary add i32 %v1, 1
    store volatile typed i32 %a1, $value
    %stop1 = cmp eq i32 %a1, -1
    branch %stop1, ^done, ^hot2
  block ^hot2:
    %v2 = load volatile typed i32 $value
    %a2 = binary add i32 %v2, 1
    store volatile typed i32 %a2, $value
    %stop2 = cmp eq i32 %a2, -1
    branch %stop2, ^done, ^hot3
  block ^hot3:
    %v3 = load volatile typed i32 $value
    %a3 = binary add i32 %v3, 1
    store volatile typed i32 %a3, $value
    %stop3 = cmp eq i32 %a3, -1
    branch %stop3, ^done, ^hot4
  block ^hot4:
    %v4 = load volatile typed i32 $value
    %a4 = binary add i32 %v4, 1
    store volatile typed i32 %a4, $value
    %stop4 = cmp eq i32 %a4, -1
    branch %stop4, ^done, ^hot5
  block ^hot5:
    %v5 = load volatile typed i32 $value
    %a5 = binary add i32 %v5, 1
    store volatile typed i32 %a5, $value
    %stop5 = cmp eq i32 %a5, -1
    branch %stop5, ^done, ^hot6
  block ^hot6:
    %v6 = load volatile typed i32 $value
    %a6 = binary add i32 %v6, 1
    store volatile typed i32 %a6, $value
    %stop6 = cmp eq i32 %a6, -1
    branch %stop6, ^done, ^hot7
  block ^hot7:
    %v7 = load volatile typed i32 $value
    %a7 = binary add i32 %v7, 1
    store volatile typed i32 %a7, $value
    %stop7 = cmp eq i32 %a7, -1
    branch %stop7, ^done, ^hot8
  block ^hot8:
    %v8 = load volatile typed i32 $value
    %a8 = binary add i32 %v8, 1
    store volatile typed i32 %a8, $value
    %stop8 = cmp eq i32 %a8, -1
    branch %stop8, ^done, ^hot9
  block ^hot9:
    %v9 = load volatile typed i32 $value
    %a9 = binary add i32 %v9, 1
    store volatile typed i32 %a9, $value
    %stop9 = cmp eq i32 %a9, -1
    branch %stop9, ^done, ^hot10
  block ^hot10:
    %v10 = load volatile typed i32 $value
    %a10 = binary add i32 %v10, 1
    store volatile typed i32 %a10, $value
    %stop10 = cmp eq i32 %a10, -1
    branch %stop10, ^done, ^hot11
  block ^hot11:
    %v11 = load volatile typed i32 $value
    %a11 = binary add i32 %v11, 1
    store volatile typed i32 %a11, $value
    %next = binary add i32 %n, 1
    %again = cmp lt i32 %next, %iterations
    branch %again, ^hot0, ^done
  block ^done:
    %result = load volatile typed i32 $value
    return i32 %result
}

# A dispatch case contains a natural loop, whose backedge stays structured.
function @nested_member(%choose : i32, %limit : i32) -> i32 {
  slot $value : i32
  block ^entry:
    store volatile typed i32 0, $value
    branch %choose, ^left, ^right
  block ^left:
    %lv = load volatile typed i32 $value
    %ldone = cmp ge i32 %lv, %limit
    branch %ldone, ^done, ^inner
  block ^inner:
    %n = phi i32 [^left: 0, ^inner: %next]
    %v = load volatile typed i32 $value
    %a = binary add i32 %v, 1
    store volatile typed i32 %a, $value
    %next = binary add i32 %n, 1
    %again = cmp lt i32 %next, 2
    branch %again, ^inner, ^right
  block ^right:
    %rv = load volatile typed i32 $value
    %rn = binary add i32 %rv, 1
    store volatile typed i32 %rn, $value
    %rdone = cmp ge i32 %rn, %limit
    branch %rdone, ^done, ^left
  block ^done:
    %result = load volatile typed i32 $value
    return i32 %result
}

# Parallel PHI transfers may cycle when a local region is re-entered.
function @regional_swap(%choose : i32, %iterations : i32) -> i64 {
  block ^entry:
    branch %choose, ^left, ^right
  block ^left:
    %lx = phi i64 [^entry: 3, ^right: %ry]
    %ly = phi i64 [^entry: 7, ^right: %rx]
    %ln = phi i32 [^entry: 0, ^right: %rnext]
    %lnext = binary add i32 %ln, 1
    %ldone = cmp ge i32 %lnext, %iterations
    branch %ldone, ^left_exit, ^right
  block ^right:
    %rx = phi i64 [^entry: 3, ^left: %ly]
    %ry = phi i64 [^entry: 7, ^left: %lx]
    %rn = phi i32 [^entry: 0, ^left: %lnext]
    %rnext = binary add i32 %rn, 1
    %rdone = cmp ge i32 %rnext, %iterations
    branch %rdone, ^right_exit, ^left
  block ^left_exit:
    %ltens = binary mul i64 %lx, 10
    %lresult = binary add i64 %ltens, %ly
    return i64 %lresult
  block ^right_exit:
    %rtens = binary mul i64 %rx, 10
    %rresult = binary add i64 %rtens, %ry
    return i64 %rresult
}

# A natural loop inside one local dispatcher contains another local dispatcher.
function @nested_regions(%choose : i32, %limit : i32) -> i32 {
  slot $value : i32
  block ^entry:
    store volatile typed i32 0, $value
    branch %choose, ^left, ^right
  block ^left:
    %lv = load volatile typed i32 $value
    %ldone = cmp ge i32 %lv, %limit
    branch %ldone, ^done, ^inner
  block ^inner:
    %n = phi i32 [^left: 0, ^latch: %next]
    branch %choose, ^first, ^second
  block ^first:
    %fn = phi i32 [^inner: 0, ^second: %snext]
    %fv = load volatile typed i32 $value
    %fa = binary add i32 %fv, 1
    store volatile typed i32 %fa, $value
    %fnext = binary add i32 %fn, 1
    %fmore = cmp lt i32 %fnext, 2
    branch %fmore, ^second, ^latch
  block ^second:
    %sn = phi i32 [^inner: 0, ^first: %fnext]
    %sv = load volatile typed i32 $value
    %sa = binary add i32 %sv, 1
    store volatile typed i32 %sa, $value
    %snext = binary add i32 %sn, 1
    %smore = cmp lt i32 %snext, 2
    branch %smore, ^first, ^latch
  block ^latch:
    %next = binary add i32 %n, 1
    %again = cmp lt i32 %next, 2
    branch %again, ^inner, ^right
  block ^right:
    %rv = load volatile typed i32 $value
    %rn = binary add i32 %rv, 1
    store volatile typed i32 %rn, $value
    %rdone = cmp ge i32 %rn, %limit
    branch %rdone, ^done, ^left
  block ^done:
    %result = load volatile typed i32 $value
    return i32 %result
}

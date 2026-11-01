# The child a heap walks to next decides which element the next round loads
# and compares: as a select the next load would wait for this one. @scan's
# merge reaches no address, so it is a select, as is @chosen's, whose condition
# is a parameter with nothing to walk.
function @walk(%a : ptr, %n : i64) -> i64 [binding=strong] {
  block ^entry:
    jump ^loop

  block ^loop:
    %j = phi i64 [^entry: 1, ^merge: %doubled]
    %more = cmp lt i64 %j, %n
    branch %more, ^body, ^done

  block ^body:
    %left_at = index f64 [projection=array_element] %a, %j
    %left = load typed f64 %left_at
    %right_index = binary add i64 %j, 1
    %right_at = index f64 [projection=array_element] %a, %right_index
    %right = load typed f64 %right_at
    %greater = cmp gt f64 %right, %left
    branch %greater, ^take_right, ^merge

  block ^take_right:
    jump ^merge

  block ^merge:
    %child = phi i64 [^body: %j, ^take_right: %right_index]
    %doubled = binary shl i64 %child, 1
    jump ^loop

  block ^done:
    return i64 %j
}

function @scan(%a : ptr, %n : i64) -> i64 [binding=strong] {
  block ^entry:
    jump ^loop

  block ^loop:
    %i = phi i64 [^entry: 0, ^merge: %next]
    %best = phi i64 [^entry: 0, ^merge: %kept]
    %more = cmp lt i64 %i, %n
    branch %more, ^body, ^done

  block ^body:
    %at = index f64 [projection=array_element] %a, %i
    %value = load typed f64 %at
    %positive = cmp gt f64 %value, 0
    branch %positive, ^take, ^merge

  block ^take:
    jump ^merge

  block ^merge:
    %kept = phi i64 [^body: %best, ^take: %i]
    %next = binary add i64 %i, 1
    jump ^loop

  block ^done:
    return i64 %best
}

function @chosen(%on : i1, %a : i64, %b : i64) -> i64 [binding=strong] {
  block ^entry:
    branch %on, ^take, ^merge

  block ^take:
    jump ^merge

  block ^merge:
    %kept = phi i64 [^entry: %a, ^take: %b]
    return i64 %kept
}

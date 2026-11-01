# A loop that stops at a runtime bound has a final value too, if the bound
# holds off the zero-trip case: it runs only from before the bound, and
# otherwise leaves its start.
function @upward(%n : i32) -> i32 [binding=strong] {
  block ^entry:
    jump ^loop

  block ^loop:
    %i = phi i32 [^entry: 0, ^body: %next]
    %sum = phi i32 [^entry: 7, ^body: %added]
    %more = cmp lt i32 %i, %n
    branch %more, ^body, ^done

  block ^body:
    %added = binary add i32 %sum, 3
    %next = binary add nowrap i32 %i, 1
    jump ^loop

  block ^done:
    return i32 %sum
}

# The induction read after the loop is the start if the loop never ran.
function @tested_read(%from : i32, %to : i32) -> i32 [binding=strong] {
  block ^entry:
    jump ^loop

  block ^loop:
    %i = phi i32 [^entry: %from, ^body: %next]
    %more = cmp lt i32 %i, %to
    branch %more, ^body, ^done

  block ^body:
    %next = binary add i32 %i, 1
    jump ^loop

  block ^done:
    return i32 %i
}

function @downward(%from : i32, %to : i32) -> i32 [binding=strong] {
  block ^entry:
    jump ^loop

  block ^loop:
    %i = phi i32 [^entry: %from, ^body: %next]
    %sum = phi i32 [^entry: 0, ^body: %added]
    %more = cmp gt i32 %i, %to
    branch %more, ^body, ^done

  block ^body:
    %added = binary add i32 %sum, 5
    %next = binary add i32 %i, -1
    jump ^loop

  block ^done:
    return i32 %sum
}

function @unsigned_bound(%n : i64) -> i64 [binding=strong] {
  block ^entry:
    jump ^loop

  block ^loop:
    %i = phi i64 [^entry: 2, ^body: %next]
    %sum = phi i64 [^entry: 0, ^body: %added]
    %more = cmp ult i64 %i, %n
    branch %more, ^body, ^done

  block ^body:
    %added = binary add i64 %sum, 4
    %next = binary add i64 %i, 1
    jump ^loop

  block ^done:
    return i64 %sum
}

# Leaving on the true side of the test, and entered from a branch, so the
# loop has no preheader until this pass gives it one.
function @leaves_on_true(%flag : i64, %n : i32) -> i8 [binding=strong] {
  block ^entry:
    branch %flag, ^loop, ^other

  block ^loop:
    %i = phi i32 [^entry: 0, ^body: %next]
    %sum = phi i8 [^entry: 0, ^body: %narrow]
    %leave = cmp ge i32 %i, %n
    branch %leave, ^done, ^body

  block ^body:
    %wide = convert sext i32 i8 %sum
    %added = binary add nowrap i32 %wide, 3
    %narrow = convert trunc i8 i32 %added
    %next = binary add nowrap i32 %i, 1
    jump ^loop

  block ^other:
    return i8 1

  block ^done:
    return i8 %sum
}

# Past the bound's reach: `le` need not end, and a step of two can jump the
# bound. Neither has a final value.
function @inclusive_bound(%n : i32) -> i32 [binding=strong] {
  block ^entry:
    jump ^loop

  block ^loop:
    %i = phi i32 [^entry: 0, ^body: %next]
    %sum = phi i32 [^entry: 0, ^body: %added]
    %more = cmp le i32 %i, %n
    branch %more, ^body, ^done

  block ^body:
    %added = binary add i32 %sum, 3
    %next = binary add i32 %i, 1
    jump ^loop

  block ^done:
    return i32 %sum
}

function @step_of_two(%n : i32) -> i32 [binding=strong] {
  block ^entry:
    jump ^loop

  block ^loop:
    %i = phi i32 [^entry: 0, ^body: %next]
    %sum = phi i32 [^entry: 0, ^body: %added]
    %more = cmp lt i32 %i, %n
    branch %more, ^body, ^done

  block ^body:
    %added = binary add i32 %sum, 3
    %next = binary add i32 %i, 2
    jump ^loop

  block ^done:
    return i32 %sum
}

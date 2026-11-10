# A chain of loads and arithmetic leaves the loop whole: the table pointer,
# its index, the row they select, then the row's stride.
# A load past a branch moves too: without calls, every pass reaches it.
global @table : ptr = 0

function @chain(%this : ptr, %out : ptr, %n : i64) -> void [binding=strong] {
  block ^entry:
    jump ^body

  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %cells = load typed ptr @table
    %at = index i8 [projection=field] %this, 8
    %which = load typed i64 %at
    %row = binary mul i64 %which, 88
    %entry = index i8 %cells, %row
    %field = index i8 [projection=field] %entry, 28
    %stride = load typed i32 %field
    %wide = convert sext i64 i32 %stride
    %step = binary mul i64 %i, %wide
    %slot = index f64 %out, %step
    store typed f64 1, %slot
    %next = binary add i64 %i, 1
    %more = cmp lt i64 %next, %n
    branch %more, ^body, ^done

  block ^done:
    return void
}

function @past_branch(%this : ptr, %out : ptr, %n : i64, %c : i64) -> void [binding=strong] {
  block ^entry:
    jump ^body

  block ^body:
    %i = phi i64 [^entry: 0, ^join: %next]
    %odd = binary and i64 %i, %c
    %test = cmp ne i64 %odd, 0
    branch %test, ^then, ^join

  block ^then:
    %slot = index f64 %out, %i
    store typed f64 2, %slot
    jump ^join

  block ^join:
    %at = index i8 [projection=field] %this, 8
    %base = load typed ptr %at
    %cell = index f64 %base, %i
    store typed f64 3, %cell
    %next = binary add i64 %i, 1
    %more = cmp lt i64 %next, %n
    branch %more, ^body, ^done

  block ^done:
    return void
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}

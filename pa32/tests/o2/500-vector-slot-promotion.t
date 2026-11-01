# A slot of v128 promotes like a scalar slot, with a phi of v128 at the join.
function @pick(%a : v128, %b : v128, %which : i32) -> v128 {
  slot $merged : v128

  block ^entry:
    %test = cmp ne i32 %which, 0
    branch %test, ^left, ^right

  block ^left:
    store v128 %a, $merged
    jump ^done

  block ^right:
    store v128 %b, $merged
    jump ^done

  block ^done:
    %value = load v128 $merged
    return v128 %value
}

function @sum_of_lanes(%count : i32) -> f64 {
  slot $total : v128

  block ^entry:
    %start = vector pzero
    store v128 %start, $total
    %one = vector fromf64 1.5
    %step = vector unpcklpd %one, %one
    jump ^loop

  block ^loop:
    %i = phi i32 [^entry: 0, ^body: %next]
    %more = cmp lt i32 %i, %count
    branch %more, ^body, ^done

  block ^body:
    %old = load v128 $total
    %new = vector addpd %old, %step
    store v128 %new, $total
    %next = binary add nowrap i32 %i, 1
    jump ^loop

  block ^done:
    %final = load v128 $total
    %high = vector unpckhpd %final, %final
    %low = vector addsd %final, %high
    %result = vector tof64 %low
    return f64 %result
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %one = vector fromi32 7
    %two = vector fromi32 11
    %picked = call v128 @pick(%one, %two, 1)
    %other = call v128 @pick(%one, %two, 0)
    %left = vector toi32 %picked
    %right = vector toi32 %other
    %sum = call f64 @sum_of_lanes(4)
    %whole = convert fptosi i32 f64 %sum
    %mix = binary add i32 %left, %right
    %total = binary add i32 %mix, %whole
    return i32 %total
}

# Repeated updates of an indexed local object across nested loops.
# Every innermost iteration stores the same snapshot plus one.
function @nested(%rounds : i64, %repeats : i64) -> i64 {
  slot $state : obj<24x16>
  block ^entry:
    jump ^initialize
  block ^initialize:
    %base = addr $state
    zeroinit 24x8 %base
    jump ^round_header

  block ^round_header:
    %round = phi i64 [^initialize: %rounds, ^element_latch: %next_round]
    branch %round, ^round_body, ^exit

  block ^round_body:
    jump ^element_header

  block ^element_header:
    %element = phi i64 [^element_latch: %next_element, ^round_body: 0]
    %narrow_element = convert trunc exact i32 i64 %element
    %address = index i64 [projection=array_element] %base, %element
    %selector = binary and i32 %narrow_element, 3
    jump ^repeat_header

  block ^repeat_header:
    %iteration = phi i64 [^repeat_latch: %next_iteration, ^element_header: 0]
    switch %selector, ^inner_guard, 0:^inner_guard, 1:^inner_guard, 2:^other_guard

  block ^other_guard:
    branch %repeats, ^other_loop, ^repeat_latch

  block ^other_loop:
    %alt = phi i64 [^other_guard: 0, ^other_loop: %alt_next]
    %alt_next = binary add i64 %alt, 1
    %alt_more = cmp ult i64 %alt_next, %repeats
    branch %alt_more, ^other_loop, ^repeat_latch

  block ^inner_guard:
    branch %repeats, ^inner_entry, ^repeat_latch

  block ^inner_entry:
    %saved = load typed i64 %address
    jump ^inner_loop

  block ^inner_loop:
    %k = phi i64 [^inner_entry: 0, ^inner_loop: %next_k]
    %update = binary add i64 %saved, 1
    store typed i64 %update, %address
    %next_k = binary add i64 %k, 1
    %more_inner = cmp ult i64 %next_k, %repeats
    branch %more_inner, ^inner_loop, ^repeat_latch

  block ^repeat_latch:
    %next_iteration = binary add i64 %iteration, 1
    %more_iterations = cmp ult i64 %next_iteration, 4
    branch %more_iterations, ^repeat_header, ^element_latch

  block ^element_latch:
    %next_element = binary add nowrap i64 %element, 1
    %narrow_next = convert trunc exact i32 i64 %next_element
    %more_elements = cmp lt i32 %narrow_next, 2
    %next_round = binary sub i64 %round, 1
    branch %more_elements, ^element_header, ^round_header

  block ^exit:
    %state0 = load i64 %base
    %p1 = index i64 %base, 1
    %state1 = load i64 %p1
    %result = binary add i64 %state0, %state1
    return i64 %result
}

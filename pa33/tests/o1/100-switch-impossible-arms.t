# Impossible switch arms must not enlarge the work for defined inputs.
global @seed : i64 = 882731
global @events : i64 = 0

function @dense(%key : i64, %value : i64) -> i64 {
  block ^entry:
    switch %key, ^dead, 0:^case0, 1:^case1, 2:^case2, 3:^case3, 4:^case4, 5:^case5, -9223372036854775808:^dead, 9223372036854775807:^dead, 0:^dead

  block ^case0:
    %r0 = binary mul i64 %value, 3
    return i64 %r0

  block ^case1:
    %r1 = binary mul i64 %value, 5
    return i64 %r1

  block ^case2:
    %r2 = binary mul i64 %value, 7
    return i64 %r2

  block ^case3:
    %r3 = binary mul i64 %value, 9
    return i64 %r3

  block ^case4:
    %r4 = binary mul i64 %value, 11
    return i64 %r4

  block ^case5:
    %r5 = binary mul i64 %value, 13
    return i64 %r5

  block ^dead:
    unreachable
}

function @signed_byte(%key : i8, %value : i64) -> i64 {
  block ^entry:
    switch %key, ^dead, -6:^case0, -5:^case1, -4:^case2, -3:^case3, -2:^case4, -1:^case5, -128:^dead, 127:^dead

  block ^case0:
    %r0 = binary mul i64 %value, 3
    return i64 %r0

  block ^case1:
    %r1 = binary mul i64 %value, 5
    return i64 %r1

  block ^case2:
    %r2 = binary mul i64 %value, 7
    return i64 %r2

  block ^case3:
    %r3 = binary mul i64 %value, 9
    return i64 %r3

  block ^case4:
    %r4 = binary mul i64 %value, 11
    return i64 %r4

  block ^case5:
    %r5 = binary mul i64 %value, 13
    return i64 %r5

  block ^dead:
    unreachable
}

function @unsigned_byte(%key : u8, %value : i64) -> i64 {
  block ^entry:
    switch %key, ^dead, 250:^case0, 251:^case1, 252:^case2, 253:^case3, 254:^case4, 255:^case5, 0:^dead, 249:^dead

  block ^case0:
    %r0 = binary mul i64 %value, 3
    return i64 %r0

  block ^case1:
    %r1 = binary mul i64 %value, 5
    return i64 %r1

  block ^case2:
    %r2 = binary mul i64 %value, 7
    return i64 %r2

  block ^case3:
    %r3 = binary mul i64 %value, 9
    return i64 %r3

  block ^case4:
    %r4 = binary mul i64 %value, 11
    return i64 %r4

  block ^case5:
    %r5 = binary mul i64 %value, 13
    return i64 %r5

  block ^dead:
    unreachable
}

function @defined_default(%key : i64, %value : i64) -> i64 {
  block ^entry:
    switch %key, ^record, 0:^case0, 1:^case1, 2:^case2, 3:^case3, 4:^case4, 5:^case5, -9223372036854775808:^dead, 9223372036854775807:^dead

  block ^case0:
    %r0 = binary mul i64 %value, 3
    return i64 %r0

  block ^case1:
    %r1 = binary mul i64 %value, 5
    return i64 %r1

  block ^case2:
    %r2 = binary mul i64 %value, 7
    return i64 %r2

  block ^case3:
    %r3 = binary mul i64 %value, 9
    return i64 %r3

  block ^case4:
    %r4 = binary mul i64 %value, 11
    return i64 %r4

  block ^case5:
    %r5 = binary mul i64 %value, 13
    return i64 %r5

  block ^dead:
    unreachable

  block ^record:
    %result = call i64 @record_default(%value)
    return i64 %result
}

function @throwing_default(%key : i64, %value : i64) -> i64 {
  block ^entry:
    switch %key, ^raise, 0:^case0, 1:^case1, 2:^case2, 3:^case3, 4:^case4, 5:^case5, -9223372036854775808:^dead, 9223372036854775807:^dead

  block ^case0:
    %r0 = binary mul i64 %value, 3
    return i64 %r0

  block ^case1:
    %r1 = binary mul i64 %value, 5
    return i64 %r1

  block ^case2:
    %r2 = binary mul i64 %value, 7
    return i64 %r2

  block ^case3:
    %r3 = binary mul i64 %value, 9
    return i64 %r3

  block ^case4:
    %r4 = binary mul i64 %value, 11
    return i64 %r4

  block ^case5:
    %r5 = binary mul i64 %value, 13
    return i64 %r5

  block ^dead:
    unreachable

  block ^raise:
    call void @raise_default(%value)
    unreachable
}

function @all_dead_cases(%key : i64, %value : i64) -> i64 {
  block ^entry:
    switch %key, ^otherwise, 0:^dead, 1:^dead, 2:^dead, 3:^dead, 4:^dead, 5:^dead
  block ^dead:
    unreachable
  block ^otherwise:
    %result = call i64 @record_default(%value)
    return i64 %result
}

function @record_default(%value : i64) -> i64 {
  block ^entry:
    %old = load volatile i64 @events
    %next = binary add i64 %old, 1
    store volatile i64 %next, @events
    %result = binary add i64 %value, 19
    return i64 %result
}

function @raise_default(%value : i64) -> void [return=noreturn] {
  block ^entry:
    %old = load volatile i64 @events
    %next = binary add i64 %old, 1
    store volatile i64 %next, @events
    throw i64 %value
}

function @main() -> i64 [role=entry] {
  block ^entry:
    %initial = load volatile i64 @seed
    jump ^loop
  block ^loop:
    %i = phi i64 [^entry: 0, ^repeat: %next_i]
    %state = phi i64 [^entry: %initial, ^repeat: %next_state]
    %errors = phi i64 [^entry: 0, ^repeat: %next_errors]
    %key = binary umod i64 %i, 6
    %u_key = copy i64 %key
    %twice = binary mul i64 %u_key, 2
    %factor = binary add i64 %twice, 3
    %expected = binary mul i64 %state, %factor
    %a = call i64 @dense(%key, %state)
    %signed_key = binary sub i64 %key, 6
    %s8 = convert trunc i8 i64 %signed_key
    %b = call i64 @signed_byte(%s8, %state)
    %unsigned_key = binary add i64 %key, 250
    %u8 = convert trunc u8 i64 %unsigned_key
    %c = call i64 @unsigned_byte(%u8, %state)
    %d = call i64 @defined_default(%key, %state)
    %e = call i64 @throwing_default(%key, %state)
    %fallback_key = binary add i64 %key, 20
    %fallback_expected = binary add i64 %state, 19
    %f = call i64 @defined_default(%fallback_key, %state)
    %g = call i64 @all_dead_cases(500, %state)
    %bad_a = cmp ne i64 %a, %expected
    %bad_b = cmp ne i64 %b, %expected
    %bad_c = cmp ne i64 %c, %expected
    %bad_d = cmp ne i64 %d, %expected
    %bad_e = cmp ne i64 %e, %expected
    %bad_f = cmp ne i64 %f, %fallback_expected
    %bad_g = cmp ne i64 %g, %fallback_expected
    %ab = binary or i64 %bad_a, %bad_b
    %cd = binary or i64 %bad_c, %bad_d
    %ef = binary or i64 %bad_e, %bad_f
    %abcd = binary or i64 %ab, %cd
    %efg = binary or i64 %ef, %bad_g
    %bad = binary or i64 %abcd, %efg
    %next_errors = binary or i64 %errors, %bad
    jump ^repeat
  block ^repeat:
    %product = binary mul i64 %state, 6364136223846793005
    %next_state = binary add i64 %product, 1442695040888963407
    %next_i = binary add i64 %i, 1
    %more = cmp ult i64 %next_i, 256
    branch %more, ^loop, ^finished
  block ^finished:
    branch %next_errors, ^wrong, ^try_default
  block ^wrong:
    return i64 1
  block ^try_default:
    eh_try ^handler
    %impossible = call i64 @throwing_default(400, 99)
    eh_end
    return i64 2
  block ^handler:
    %exception = exception i64
    %events = load volatile i64 @events
    %bad_exception = cmp ne i64 %exception, 99
    %bad_events = cmp ne i64 %events, 513
    %result = binary or i64 %bad_exception, %bad_events
    return i64 %result
}

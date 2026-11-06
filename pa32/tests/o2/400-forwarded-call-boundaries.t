global @input : i64 = 511

function @relay(%a : i64, %b : i64, %depth : i64) -> i64 {
  block ^entry:
    %r = call i64 @permuted(%depth, %b, %a)
    return i64 %r
}

function @permuted(%depth : i64, %b : i64, %a : i64) -> i64 {
  block ^entry:
    %r = call i64 @worker(%depth, %a, %b)
    return i64 %r
}

function @constant_depth(%a : i64, %b : i64, %unused : i64) -> i64 {
  block ^entry:
    %r = call i64 @worker(0, %a, %b)
    return i64 %r
}

function @worker(%depth : i64, %a : i64, %b : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %zero = cmp eq i64 %depth, 0
    branch %zero, ^base, ^nonzero
  block ^base:
    %answer = binary xor i64 %a, %b
    return i64 %answer
  block ^nonzero:
    %one = cmp eq i64 %depth, 1
    branch %one, ^constant, ^step
  block ^constant:
    %last = call i64 @constant_depth(%a, %b, %depth)
    return i64 %last
  block ^step:
    %next = binary sub i64 %depth, 1
    %nested = call i64 @relay(%b, %a, %next)
    %total = binary add i64 %nested, %a
    return i64 %total
}

function @held(%n : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %r = call i64 @held_worker(%n)
    return i64 %r
}

function @held_worker(%n : i64) -> i64 {
  block ^entry:
    %zero = cmp eq i64 %n, 0
    branch %zero, ^base, ^step
  block ^base:
    return i64 0
  block ^step:
    %next = binary sub i64 %n, 1
    %r = call i64 @held(%next)
    %answer = binary add i64 %r, 1
    return i64 %answer
}

function @narrow_target(%x : u8) -> i64 [no_inline=yes] {
  block ^entry:
    %wide = convert zext i64 u8 %x
    return i64 %wide
}

function @narrow_wrapper(%x : u8) -> i64 {
  block ^entry:
    %r = call i64 @narrow_target(%x)
    return i64 %r
}

function @wide_caller(%x : i64) -> i64 {
  block ^entry:
    %small = convert trunc u8 i64 %x
    %r = call i64 @narrow_wrapper(%small)
    return i64 %r
}

# A cycle consisting only of forwarding calls has no terminating target.
# It is deliberately not entered by the behavior driver.
function @cycle_a(%x : i64) -> i64 {
  block ^entry:
    %r = call i64 @cycle_b(%x)
    return i64 %r
}

function @cycle_b(%x : i64) -> i64 {
  block ^entry:
    %r = call i64 @cycle_a(%x)
    return i64 %r
}

function @main() -> i64 [role=entry] {
  block ^entry:
    %a = call i64 @worker(4, 7, 11)
    %a_ok = cmp eq i64 %a, 37
    branch %a_ok, ^held, ^failed
  block ^held:
    %b = call i64 @held_worker(9)
    %b_ok = cmp eq i64 %b, 9
    branch %b_ok, ^width, ^failed
  block ^width:
    %raw = load volatile i64 @input
    %c = call i64 @wide_caller(%raw)
    %c_ok = cmp eq i64 %c, 255
    branch %c_ok, ^passed, ^failed
  block ^passed:
    return i64 0
  block ^failed:
    return i64 1
}

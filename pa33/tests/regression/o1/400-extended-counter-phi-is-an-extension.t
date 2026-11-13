# A loop rotated around a `u32` counter that may wrap leaves its extension a
# phi of its own: every input of `%wide` is the extension of the input `%i`
# takes on that edge. `@fill` keeps one phi, and `%wide` becomes its
# extension. In `@steady` the counter never wraps (`nowrap`): the pair stays
# for the loop passes, which step the wide counter with the narrow one.
function @fill(%from : ptr, %to : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    %any = cmp ne i64 %n, 0
    branch %any, ^body, ^done

  block ^body:
    %i = phi u32 [^entry: 0, ^body: %next]
    %wide = phi i64 [^entry: 0, ^body: %extended]
    %source = index u32 %from, %wide
    %at = load u32 %source
    %slot = convert zext i64 u32 %at
    %target = index u32 %to, %slot
    store u32 %i, %target
    %next = binary add u32 %i, 1
    %extended = convert zext i64 u32 %next
    %more = cmp ult i64 %extended, %n
    branch %more, ^body, ^done

  block ^done:
    return void
}

function @steady(%from : ptr, %n : i64) -> i32 [no_inline=yes] {
  block ^entry:
    %any = cmp ne i64 %n, 0
    branch %any, ^body, ^done

  block ^body:
    %i = phi i32 [^entry: 0, ^body: %next]
    %wide = phi i64 [^entry: 0, ^body: %extended]
    %sum = phi i32 [^entry: 0, ^body: %total]
    %source = index i32 %from, %wide
    %value = load i32 %source
    %total = binary add i32 %sum, %value
    %next = binary add nowrap i32 %i, 1
    %extended = convert sext i64 i32 %next
    %more = cmp ult i64 %extended, %n
    branch %more, ^body, ^done

  block ^done:
    %result = phi i32 [^entry: 0, ^body: %total]
    return i32 %result
}

global @order : obj<16x4> [binding=internal] = {
  u32 3
  u32 2
  u32 1
  u32 0
}

global @places : obj<16x4> [binding=internal] = {
  u32 9
  u32 9
  u32 9
  u32 9
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %order = addr @order
    %places = addr @places
    call void @fill(%order, %places, 4)
    %first = load u32 %places
    %sum = call i32 @steady(%places, 4)
    %signed = copy i32 %first
    %scaled = binary mul i32 %signed, 10
    %result = binary add i32 %scaled, %sum
    return i32 %result
}

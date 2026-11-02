function @byte_of_extension(%first : ptr, %count : i32, %v : i8) -> i8 [binding=strong, no_inline=yes] {
  block ^entry:
    %wide = convert sext i32 i8 %v
    %more = cmp gt i32 %count, 0
    branch %more, ^loop, ^done

  block ^loop:
    %sum = phi i8 [^entry: 0, ^loop: %next_sum]
    %i = phi i32 [^entry: 0, ^loop: %next]
    %p = index i8 %first, %i
    %x = load i8 %p
    %ex = convert sext i32 i8 %x
    %a = binary add nowrap i32 %wide, %ex
    %es = convert sext i32 i8 %sum
    %b = binary add nowrap i32 %a, %es
    %next_sum = convert trunc i8 i32 %b
    %next = binary add nowrap i32 %i, 1
    %again = cmp lt i32 %next, %count
    branch %again, ^loop, ^done

  block ^done:
    %r = phi i8 [^entry: 0, ^loop: %next_sum]
    return i8 %r
}

function @byte_of_parameter(%first : ptr, %count : i32, %v : i32) -> i8 [binding=strong, no_inline=yes] {
  block ^entry:
    %more = cmp gt i32 %count, 0
    branch %more, ^loop, ^done

  block ^loop:
    %sum = phi i8 [^entry: 0, ^loop: %next_sum]
    %i = phi i32 [^entry: 0, ^loop: %next]
    %p = index i8 %first, %i
    %x = load i8 %p
    %ex = convert sext i32 i8 %x
    %a = binary add nowrap i32 %v, %ex
    %es = convert sext i32 i8 %sum
    %b = binary add nowrap i32 %a, %es
    %next_sum = convert trunc i8 i32 %b
    %next = binary add nowrap i32 %i, 1
    %again = cmp lt i32 %next, %count
    branch %again, ^loop, ^done

  block ^done:
    %r = phi i8 [^entry: 0, ^loop: %next_sum]
    return i8 %r
}

function @short_of_phi(%first : ptr, %count : i32, %c : i1, %v : i32, %w : i32) -> i16 [binding=strong, no_inline=yes] {
  block ^entry:
    %more = cmp gt i32 %count, 0
    branch %c, ^one, ^two

  block ^one:
    %v2 = binary add i32 %v, 3
    jump ^join

  block ^two:
    jump ^join

  block ^join:
    %m = phi i32 [^one: %v2, ^two: %w]
    branch %more, ^loop, ^done

  block ^loop:
    %sum = phi i16 [^join: 0, ^loop: %next_sum]
    %i = phi i32 [^join: 0, ^loop: %next]
    %p = index i16 %first, %i
    %x = load typed i16 %p
    %ex = convert sext i32 i16 %x
    %a = binary mul nowrap i32 %m, %ex
    %es = convert sext i32 i16 %sum
    %b = binary add nowrap i32 %a, %es
    %next_sum = convert trunc i16 i32 %b
    %next = binary add nowrap i32 %i, 1
    %again = cmp lt i32 %next, %count
    branch %again, ^loop, ^done

  block ^done:
    %r = phi i16 [^join: 0, ^loop: %next_sum]
    return i16 %r
}

function @byte_shift_stays(%first : ptr, %count : i32, %v : i32) -> i8 [binding=strong, no_inline=yes] {
  block ^entry:
    %more = cmp gt i32 %count, 0
    branch %more, ^loop, ^done

  block ^loop:
    %sum = phi i8 [^entry: 0, ^loop: %next_sum]
    %i = phi i32 [^entry: 0, ^loop: %next]
    %p = index i8 %first, %i
    %x = load i8 %p
    %ex = convert sext i32 i8 %x
    %a = binary shr i32 %ex, %v
    %es = convert sext i32 i8 %sum
    %b = binary add nowrap i32 %a, %es
    %next_sum = convert trunc i8 i32 %b
    %next = binary add nowrap i32 %i, 1
    %again = cmp lt i32 %next, %count
    branch %again, ^loop, ^done

  block ^done:
    %r = phi i8 [^entry: 0, ^loop: %next_sum]
    return i8 %r
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}

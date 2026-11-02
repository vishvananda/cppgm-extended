# A value read last by an immediate shift keeps its register, rcx included; only
# a shift by a variable count keeps its result out of rcx. A select's arm read
# last there takes the select's register, so the select moves nothing.
function @walk(%a : i64, %n : i64, %c : i64, %mask : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    jump ^head

  block ^head:
    %m = phi i64 [^entry: %mask, ^head: %next]
    %s = phi i64 [^entry: 0, ^head: %picked]
    %i = phi i64 [^entry: 0, ^head: %step]
    %twice = binary shl i64 %s, 1
    %one = binary add i64 %twice, 1
    %test = binary and i64 %m, %a
    %two = binary add i64 %twice, 2
    %picked = select i64 %test, %two, %one
    %next = binary ushr i64 %m, 1
    %step = binary add i64 %i, 1
    %more = cmp ult i64 %step, %n
    branch %more, ^head, ^done

  block ^done:
    %r = binary add i64 %picked, %c
    return i64 %r
}

function @in_rcx(%a : i64, %b : i64, %c : i64, %mask : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %half = binary ushr i64 %mask, 1
    %sum = binary add i64 %a, %b
    %more = binary add i64 %sum, %c
    %r = binary xor i64 %more, %half
    return i64 %r
}

function @variable(%x : i64, %count : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %r = binary shl i64 %x, %count
    return i64 %r
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call i64 @walk(5, 3, 1, 4)
    %b = call i64 @variable(3, 2)
    %c = call i64 @in_rcx(1, 2, 4, 8)
    %d = binary add i64 %a, %b
    %e = binary add i64 %c, %d
    %r = convert trunc i32 i64 %e
    return i32 %r
}

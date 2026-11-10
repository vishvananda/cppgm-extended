# Check defined values and live ranges under any valid instruction selection.
function @high(%a : i64, %b : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %x = convert zext i128 i64 %a
    %y = convert zext i128 i64 %b
    %p = binary mul i128 %x, %y
    %h = binary ushr i128 %p, 64
    %r = convert trunc i64 i128 %h
    return i64 %r
}

function @square(%a : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %x = convert zext i128 i64 %a
    %p = binary mul i128 %x, %x
    %h = binary ushr i128 %p, 64
    %r = convert trunc i64 i128 %h
    return i64 %r
}

function @signed(%a : i64, %b : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %x = convert sext i128 i64 %a
    %y = convert sext i128 i64 %b
    %p = binary mul i128 %x, %y
    %h = binary shr i128 %p, 64
    %r = convert trunc i64 i128 %h
    return i64 %r
}

function @literal(%a : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %x = convert zext i128 i64 %a
    %p = binary mul i128 %x, 12379400392853802749
    %h = binary ushr i128 %p, 64
    %r = convert trunc i64 i128 %h
    return i64 %r
}

function @shared(%a : i64, %b : i64, %c : i64, %f : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %x = convert zext i128 i64 %a
    %y = convert zext i128 i64 %b
    %p = binary mul i128 %x, %y
    %h = binary ushr i128 %p, 64
    %r = convert trunc i64 i128 %h
    %go = cmp ne i64 %f, 0
    branch %go, ^more, ^done

  block ^more:
    %z = convert zext i128 i64 %c
    %q = binary mul i128 %x, %z
    %k = binary ushr i128 %q, 64
    %s = convert trunc i64 i128 %k
    %t = binary add i64 %r, %s
    jump ^done

  block ^done:
    %v = phi i64 [^entry: %r, ^more: %t]
    return i64 %v
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %h = call i64 @high(-1, 6)
    %q = call i64 @square(-1)
    %s = call i64 @signed(-1, 6)
    %l = call i64 @literal(-1)
    %m = call i64 @shared(-1, 6, 10, 1)
    %bad_h = cmp ne i64 %h, 5
    %bad_q = cmp ne i64 %q, -2
    %bad_s = cmp ne i64 %s, -1
    %bad_l = cmp ne i64 %l, 12379400392853802748
    %bad_m = cmp ne i64 %m, 14
    %a = convert zext i64 i1 %bad_h
    %b = convert zext i64 i1 %bad_q
    %c = convert zext i64 i1 %bad_s
    %d = convert zext i64 i1 %bad_l
    %e = convert zext i64 i1 %bad_m
    %ab = binary or i64 %a, %b
    %abc = binary or i64 %ab, %c
    %abcd = binary or i64 %abc, %d
    %all = binary or i64 %abcd, %e
    %status = convert trunc i32 i64 %all
    return i32 %status
}

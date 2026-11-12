# Copies through temporary objects too large to split into fields (160
# bytes): a temporary that one copy fills and one copy reads goes, and the
# reader takes the first copy's source. Two chains run side by side (the two
# ends of a range), and a by-value copy (160 bytes) alternates with a member
# copy (153 bytes, without the tail padding): each forwarded copy moves the
# bytes both of its copies cover. A write that may reach a waiting copy's
# source keeps that copy.
global @first : obj<160x8> [binding=internal] = {
  i64 1
  i64 2
  i64 3
  i64 4
  zero 120
  u8 5
  zero 7
}
global @second : obj<160x8> [binding=internal] = {
  i64 10
  i64 20
  i64 30
  i64 40
  zero 120
  u8 50
  zero 7
}
global @sum : i64 = 0

function @take(%p : ptr, %q : ptr) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %a = load i64 %p
    %b = load i64 %q
    %p3 = index i8 %p, 24
    %c = load i64 %p3
    %q4 = index i8 %q, 152
    %d = load u8 %q4
    %dw = convert zext i64 u8 %d
    %s0 = load i64 @sum
    %s1 = binary add i64 %s0, %a
    %s2 = binary add i64 %s1, %b
    %s3 = binary add i64 %s2, %c
    %s4 = binary add i64 %s3, %dw
    store i64 %s4, @sum
    return void
}

# Two objects passed on together through two temporaries each: one copy
# from each global is left.
function @range() -> void [binding=strong, no_inline=yes] {
  slot $x : obj<160x8>
  slot $y : obj<160x8>
  slot $xm : obj<160x8>
  slot $ym : obj<160x8>
  slot $begin : obj<160x8>
  slot $end : obj<160x8>
  block ^entry:
    %x = addr $x
    copyobj 160x8 @first, %x
    %y = addr $y
    copyobj 160x8 @second, %y
    %xm = addr $xm
    copyobj 153x8 %x, %xm
    %ym = addr $ym
    copyobj 153x8 %y, %ym
    %b = addr $begin
    copyobj 160x8 %xm, %b
    %e = addr $end
    copyobj 160x8 %ym, %e
    call void @take(%b, %e)
    return void
}

# A store into the source while its copy waits: the copy stays.
function @written() -> void [binding=strong, no_inline=yes] {
  slot $x : obj<160x8>
  slot $begin : obj<160x8>
  block ^entry:
    %x = addr $x
    copyobj 160x8 @first, %x
    store i64 100, @first
    %b = addr $begin
    copyobj 160x8 %x, %b
    store i64 1, @first
    call void @take(%b, %b)
    return void
}

# A call while the copy of a global waits: the copy stays.
function @called() -> void [binding=strong, no_inline=yes] {
  slot $x : obj<160x8>
  slot $begin : obj<160x8>
  block ^entry:
    %x = addr $x
    copyobj 160x8 @first, %x
    call void @take(@first, @second)
    %b = addr $begin
    copyobj 160x8 %x, %b
    call void @take(%b, %b)
    return void
}

function @main() -> i32 [role=entry] {
  block ^entry:
    call void @range()
    call void @written()
    call void @called()
    %s = load i64 @sum
    %t = convert trunc i32 i64 %s
    %u = binary and i32 %t, 255
    return i32 %u
}

# Check defined values and live ranges under any valid instruction selection.
function @by_seven(%x : i32) -> i32 [no_inline=yes] {
  block ^entry:
    %r = binary div i32 %x, 7
    return i32 %r
}

function @by_minus_seven(%x : i32) -> i32 [no_inline=yes] {
  block ^entry:
    %r = binary div i32 %x, -7
    return i32 %r
}

function @rest_of(%x : i32) -> i32 [no_inline=yes] {
  block ^entry:
    %r = binary mod i32 %x, 139968
    return i32 %r
}

function @unsigned_by_ten(%x : i32) -> i32 [no_inline=yes] {
  block ^entry:
    %r = binary udiv i32 %x, 10
    return i32 %r
}

function @unsigned_by_seven(%x : i32) -> i32 [no_inline=yes] {
  block ^entry:
    %r = binary udiv i32 %x, 7
    return i32 %r
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call i32 @by_seven(-50)
    %b = call i32 @by_minus_seven(50)
    %c = call i32 @rest_of(200000)
    %d = call i32 @unsigned_by_ten(-1)
    %e = call i32 @unsigned_by_seven(-1)
    %s = binary add i32 %a, %b
    %t = binary add i32 %s, %c
    %u = binary add i32 %t, %d
    %v = binary add i32 %u, %e
    %w = binary mod i32 %v, 100
    return i32 %w
}

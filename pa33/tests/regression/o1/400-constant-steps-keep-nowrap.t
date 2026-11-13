# Two steps by constants that never wrap (`nowrap`) merge into one step that
# keeps the fact when the constants' sum is a value of the type: `@merged`,
# `@mixed` and `@word` keep it. `@past` adds constants whose sum is not an
# i32, `@plain` has a step without the fact, and `@down` would step an
# unsigned word by a negative sum: none of these keeps it.
function @merged(%x : i32) -> i32 {
  block ^entry:
    %a = binary add nowrap i32 %x, 1
    %b = binary add nowrap i32 %a, 1
    return i32 %b
}
function @mixed(%x : i32) -> i32 {
  block ^entry:
    %a = binary add nowrap i32 %x, 5
    %b = binary sub nowrap i32 %a, 2
    return i32 %b
}
function @word(%x : u32) -> u32 {
  block ^entry:
    %a = binary add nowrap u32 %x, 3
    %b = binary add nowrap u32 %a, 4
    return u32 %b
}
function @past(%x : i32) -> i32 {
  block ^entry:
    %a = binary add nowrap i32 %x, 2147483647
    %b = binary add nowrap i32 %a, 1
    return i32 %b
}
function @plain(%x : i32) -> i32 {
  block ^entry:
    %a = binary add i32 %x, 1
    %b = binary add nowrap i32 %a, 1
    return i32 %b
}
function @down(%x : u32) -> u32 {
  block ^entry:
    %a = binary sub nowrap u32 %x, 5
    %b = binary add nowrap u32 %a, 2
    return u32 %b
}

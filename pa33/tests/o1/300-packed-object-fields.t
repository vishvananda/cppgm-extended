# Register-sized object fields may pack at O1. Whole-word reads must see all
# narrow stores; a volatile field still requires its observable memory access.
function @packed(%a : u8, %b : u8, %c : u16, %d : u32) -> i64 [no_inline=yes] {
  slot $value : obj<8x8>
  block ^entry:
    %base = addr $value
    %at1 = index i8 [projection=field] %base, 1
    %at2 = index i8 [projection=field] %base, 2
    %at4 = index i8 [projection=field] %base, 4
    store u8 %a, %base
    store u8 %b, %at1
    store u16 %c, %at2
    store u32 %d, %at4
    %bits = load i64 %base
    return i64 %bits
}

function @volatile_field(%a : i64) -> i64 [no_inline=yes] {
  slot $value : obj<8x8>
  block ^entry:
    %base = addr $value
    store volatile i64 %a, %base
    %bits = load volatile i64 %base
    return i64 %bits
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %bits = call i64 @packed(1, 2, 3, 4)
    %ok = cmp eq i64 %bits, 17180066305
    %observed = call i64 @volatile_field(-17)
    %same = cmp eq i64 %observed, -17
    %all = binary and i1 %ok, %same
    branch %all, ^pass, ^fail
  block ^pass:
    return i32 0
  block ^fail:
    return i32 1
}

target wasm32 1

function @signed_bit() -> i32 {
  block ^entry:
    %bit = const i1 1
    %result = convert sext i32 i1 %bit
    return i32 %result
}

function @unsigned_byte_float() -> f64 {
  block ^entry:
    %byte = const i8 -1
    %result = convert uitofp f64 i8 %byte
    return f64 %result
}

function @unsigned_narrow_division(%input : i32) -> i32 {
  block ^entry:
    %byte = convert trunc i8 i32 %input
    %quotient = binary udiv i8 %byte, 2
    %result = convert zext i32 i8 %quotient
    return i32 %result
}

function @unsigned_narrow_shift(%input : i32) -> i32 {
  block ^entry:
    %byte = convert trunc i8 i32 %input
    %shifted = binary ushr i8 %byte, 1
    %result = convert zext i32 i8 %shifted
    return i32 %result
}

function @signed_unsigned_storage(%input : i32) -> i32 {
  block ^entry:
    %byte = convert trunc u8 i32 %input
    %quotient = binary div u8 %byte, 2
    %result = convert sext i32 u8 %quotient
    return i32 %result
}

function @unsigned_narrow_compare(%input : i32) -> i64 {
  block ^entry:
    %byte = convert trunc i8 i32 %input
    %result = cmp ult i8 %byte, 200
    return i64 %result
}

target wasm32 1

global @mark : i32 = 17

function @narrow() -> i32 {
 block ^entry:
  eh_try ^handler
  throw i8 -17
 block ^handler:
  %value = exception i8
  %wide = convert sext i32 i8 %value
  return i32 %wide
}

function @float_bits() -> i32 {
 block ^entry:
  eh_try ^handler
  throw f32 -0.0f
 block ^handler:
  %value = exception i32
  return i32 %value
}

function @double_bits() -> i64 {
 block ^entry:
  eh_try ^handler
  throw f64 -0.0
 block ^handler:
  %value = exception i64
  return i64 %value
}

function @floating() -> f64 {
 block ^entry:
  eh_try ^handler
  throw f64 17.25
 block ^handler:
  %value = exception f64
  return f64 %value
}

function @pointer() -> i32 {
 block ^entry:
  eh_try ^handler
  throw ptr @mark
 block ^handler:
  %value = exception ptr
  %result = load i32 %value
  return i32 %result
}

function @wide_bits() -> i64 {
 block ^entry:
  eh_try ^handler
  throw i128 18446744073709551623
 block ^handler:
  %value = exception i128
  %match = cmp eq i128 %value, 18446744073709551623
  %result = select i64 %match, 17, 1
  return i64 %result
}

function @answer() -> i32 [unwind=no] {
 block ^entry:
  return i32 17
}

function @function_pointer() -> i32 {
 block ^entry:
  eh_try ^handler
  throw ptr @answer
 block ^handler:
  %callee = exception ptr
  %result = call i32 %callee() as () -> i32 [unwind=no]
  return i32 %result
}

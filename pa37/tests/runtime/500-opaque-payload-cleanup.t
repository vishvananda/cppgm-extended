target wasm32 1

global @count : i32 = 0

declare function @raise_foreign(%n : i64) -> void

function @raise(%n : i64) -> void {
 block ^entry:
  throw i64 %n
}

function @observe(%n : i64) -> i64 {
 block ^entry:
  store i32 0, @count
  eh_try ^handler
  eh_cleanup ^cleanup
  call void @raise(%n)
  unreachable
 block ^cleanup:
  store i32 4, @count
  resume
 block ^handler:
  eh_catch_all, 7
  %value = exception i64
  %count = load i32 @count
  %wide = convert sext i64 i32 %count
  %result = binary add i64 %value, %wide
  return i64 %result
}

function @foreign(%n : i64) -> i64 {
 block ^entry:
  eh_try ^handler
  call void @raise_foreign(%n)
  unreachable
 block ^handler:
  eh_catch_all, 11
  %value = exception i64
  %sel = exception_selector i32
  %wide = convert sext i64 i32 %sel
  %result = binary add i64 %value, %wide
  return i64 %result
}

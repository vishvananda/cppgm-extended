declare function @trap() -> void [effects=readnone, unwind=no, return=noreturn]

function @callee(%x : i64) -> i64 [effects=readonly, unwind=no, return=returns] {
  block ^entry:
    return i64 %x
}

function @helper(%fn : ptr, %x : i64) -> i64 [effects=readwrite, unwind=may, return=returns] {
  block ^entry:
    %0 = call i64 %fn(%x) as (%arg : i64) -> i64 [effects=readonly, unwind=no, return=returns]
    return i64 %0
}

function @main() -> i64 {
  block ^entry:
    %0 = call i64 @callee(7)
    return i64 %0
}

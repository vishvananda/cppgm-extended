global @small = { i64 17 i64 18 i64 19 i64 20 i64 21 i64 22 i8 23 }

function @large_zero_0(%destination : ptr) -> void {
  block ^entry:
    zeroinit 4294967295x1 %destination
    return void
}

function @large_zero_1(%destination : ptr) -> void {
  block ^entry:
    zeroinit 4294967295x1 %destination
    return void
}

function @large_zero_2(%destination : ptr) -> void {
  block ^entry:
    zeroinit 4294967295x1 %destination
    return void
}

function @large_zero_3(%destination : ptr) -> void {
  block ^entry:
    zeroinit 4294967295x1 %destination
    return void
}

function @large_zero_4(%destination : ptr) -> void {
  block ^entry:
    zeroinit 4294967295x1 %destination
    return void
}

function @large_zero_5(%destination : ptr) -> void {
  block ^entry:
    zeroinit 4294967295x1 %destination
    return void
}

function @large_zero_6(%destination : ptr) -> void {
  block ^entry:
    zeroinit 4294967295x1 %destination
    return void
}

function @large_zero_7(%destination : ptr) -> void {
  block ^entry:
    zeroinit 4294967295x1 %destination
    return void
}

function @large_zero_8(%destination : ptr) -> void {
  block ^entry:
    zeroinit 4294967295x1 %destination
    return void
}

function @large_zero_9(%destination : ptr) -> void {
  block ^entry:
    zeroinit 4294967295x1 %destination
    return void
}

function @large_zero_10(%destination : ptr) -> void {
  block ^entry:
    zeroinit 4294967295x1 %destination
    return void
}

function @large_zero_11(%destination : ptr) -> void {
  block ^entry:
    zeroinit 4294967295x1 %destination
    return void
}

function @large_zero_12(%destination : ptr) -> void {
  block ^entry:
    zeroinit 4294967295x1 %destination
    return void
}

function @large_zero_13(%destination : ptr) -> void {
  block ^entry:
    zeroinit 4294967295x1 %destination
    return void
}

function @large_zero_14(%destination : ptr) -> void {
  block ^entry:
    zeroinit 4294967295x1 %destination
    return void
}

function @large_zero_15(%destination : ptr) -> void {
  block ^entry:
    zeroinit 4294967295x1 %destination
    return void
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %address = addr @small
    zeroinit 49x1 %address
    %first = load i64 @small
    %last_address = index i8 %address, 48
    %last = load i8 %last_address
    %first_bad = cmp ne i64 %first, 0
    %last_bad = cmp ne i8 %last, 0
    branch %first_bad, ^bad, ^check_last
  block ^check_last:
    branch %last_bad, ^bad, ^ok
  block ^bad:
    return i32 1
  block ^ok:
    return i32 0
}

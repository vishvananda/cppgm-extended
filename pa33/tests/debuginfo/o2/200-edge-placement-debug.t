global @seed = {
  i64 40
}

function @identity(%value : i64) -> i64 [unwind=no] {
  block ^entry:
    return i64 %value !dbg(test.cpp, 2, 3)
}

function @main() -> i64 [role=entry] {
  block ^entry:
    %seed = addr @seed
    %input = load volatile i64 %seed
    %stable = binary add i64 %input, 2 !dbg(test.cpp, 7, 5)
    jump ^loop !dbg(test.cpp, 8, 5)

  block ^loop:
    %observed = call i64 @identity(%stable) !dbg(test.cpp, 11, 5)
    branch %observed, ^done, ^loop !dbg(test.cpp, 12, 5)

  block ^done:
    %bad = cmp ne i64 %observed, 42 !dbg(test.cpp, 15, 5)
    return i64 %bad !dbg(test.cpp, 16, 5)
}

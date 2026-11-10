global @seed = {
  i64 40
}

function @identity(%value : i64) -> i64 [unwind=no] {
  block ^entry:
    return i64 %value
}

function @main() -> i64 [role=entry] {
  block ^entry:
    %seed = addr @seed
    %input = load volatile i64 %seed
    %stable = binary add i64 %input, 2
    eh_try ^handler
    %ignored = call i64 @identity(%stable)
    eh_end
    jump ^done

  block ^done:
    %bad = cmp ne i64 %stable, 42
    return i64 %bad

  block ^handler:
    return i64 2
}

function @report() -> void [cold=yes] {
  block ^entry:
    return void
}

function @main() -> i64 [role=entry] {
  block ^entry:
    %fp = addr @report
    call void %fp() as () -> void [cold=yes]
    return i64 0
}

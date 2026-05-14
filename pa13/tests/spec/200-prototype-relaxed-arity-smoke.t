declare function @legacy() -> i64 [arity=prototype_relaxed]

function @main() -> i64 {
  block ^entry:
    %0 = const i64 0
    return i64 %0
}

target wasm32 1

function @irreducible(%choose : i32) -> i32 {
  block ^entry:
    branch %choose, ^a, ^b
  block ^a:
    %av = phi i32 [^entry: 0, ^b: %bn]
    %an = binary add i32 %av, 1
    %adone = cmp ge i32 %an, 6
    branch %adone, ^exit, ^b
  block ^b:
    %bv = phi i32 [^entry: 0, ^a: %an]
    %bn = binary add i32 %bv, 1
    %bdone = cmp ge i32 %bn, 6
    branch %bdone, ^exit, ^a
  block ^exit:
    %result = phi i32 [^a: %an, ^b: %bn]
    return i32 %result
}

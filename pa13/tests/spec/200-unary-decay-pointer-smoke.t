global @g : i64 = 4

function @main() -> i64 {
  block ^entry:
    %raw = addr @g
    %decayed = unary decay ptr %raw
    %same = cmp eq ptr %raw, %decayed
    return i64 %same
}

function @helper(%ret : ptr [pass=indirect_result], %obj : ptr [pass=by_address], %ref : ptr [pass=reference], %arr : ptr [pass=decay], %x : i64) -> void {
  block ^entry:
    store i64 %x, %ret
    return void
}

function @main() -> i64 {
  slot $out : i64

  block ^entry:
    zeroinit 8x8 $out
    call void @helper($out, $out, $out, $out, 5)
    %0 = load i64 $out
    return i64 %0
}

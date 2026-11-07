function @copy_fields(%value : v128) -> v128 {
  slot $source : obj<16x16>
  slot $target : obj<16x16>
  block ^entry:
    %from = addr $source
    %to = addr $target
    store v128 %value, %from
    copyobj 16x16 %from, %to
    %answer = load v128 %to
    return v128 %answer
}

function @zero_copied_fields() -> v128 {
  slot $source : obj<16x16>
  slot $target : obj<16x16>
  block ^entry:
    %from = addr $source
    %to = addr $target
    zeroinit 16x16 %from
    copyobj 16x16 %from, %to
    %answer = load v128 %to
    return v128 %answer
}

function @escaped_fields(%value : v128, %dest : ptr) -> void {
  slot $source : obj<16x16>
  block ^entry:
    %from = addr $source
    store v128 %value, %from
    copyobj 16x16 %from, %dest
    return void
}

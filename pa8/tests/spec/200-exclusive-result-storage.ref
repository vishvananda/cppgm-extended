function @fill(%out : ptr [pass=indirect_result, alias=exclusive_result, object_bytes=24], %input : ptr) -> void {
  block ^entry:
    %x = load i64 %input
    store i64 %x, %out
    return void
}

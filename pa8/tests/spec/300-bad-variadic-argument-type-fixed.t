declare function @sink(%n : i32, %m : i32) -> void
function @caller() -> void {
  block ^entry:
    call void @sink(1, 2) [argument_type=1:i32]
    return void
}

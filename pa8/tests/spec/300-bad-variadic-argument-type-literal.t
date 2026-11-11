declare function @sink(%n : i32) -> void [arity=variadic]
function @caller() -> void {
  block ^entry:
    call void @sink(1, 2) [argument_type=1:f64]
    return void
}

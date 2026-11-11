target wasm32 1
declare function @take(%tag : i32) -> void [arity=variadic]
function @produce(%record : obj<16x8> [shape=mixed], %quad : obj<16x16> [shape=f128x1]) -> void {
  slot $empty : obj<1x1>
  block ^entry:
    call void @take(7, %record, %quad, $empty) [argument_shape=1:mixed, argument_shape=2:f128x1, argument_shape=3:empty]
    return void
}
function @consume(%tag : i32) -> void [arity=variadic] {
  slot $list : ptr
  block ^entry:
    %pointer = addr $list
    va_start %pointer
    %record = va_arg obj<16x8> %pointer [shape=mixed]
    %quad = va_arg obj<16x16> %pointer [shape=f128x1]
    %empty = va_arg obj<1x1> %pointer [shape=empty]
    return void
}

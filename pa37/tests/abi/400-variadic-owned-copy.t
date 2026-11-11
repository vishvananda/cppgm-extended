target wasm32 1
function @change(%tag : i32) -> void [arity=variadic, no_inline=yes] {
  slot $list : ptr
  block ^entry:
    %list = addr $list
    va_start %list
    %buffer = load ptr $list
    %copy = load ptr %buffer
    store i32 99, %copy
    return void
}
function @observe() -> i32 {
  slot $source : obj<8x4>
  slot $saved : obj<8x4>
  block ^entry:
    store i32 4, $source
    %value = load obj<8x4> $source
    call void @change(0, %value) [argument_shape=1:mixed]
    store obj<8x4> %value, $saved
    %actual = load i32 $saved
    %correct = cmp eq i32 %actual, 4
    %answer = select i32 %correct, 17, 1
    return i32 %answer
}

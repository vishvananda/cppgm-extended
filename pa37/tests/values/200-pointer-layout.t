target wasm32 1

global @bytes : obj<16x4> = zero
global @address : ptr = addr @bytes

function @pointer_stride() -> i32 {
  slot $object : obj<16x4>
  block ^entry:
    %p = addr $object
    %q = index i32 %p, 1
    store i32 42, %q
    %read = load i32 %q
    %global = load ptr @address
    %same = cmp eq ptr %global, @bytes
    %check = convert trunc i32 i64 %same
    %result = binary add i32 %read, %check
    return i32 %result
}

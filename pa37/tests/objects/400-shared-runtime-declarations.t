target wasm32 1

declare function @source_move(%dst : ptr, %src : ptr, %count : u32) -> ptr [unwind=no, binding=strong, object=cppgm_builtin_memmove]
declare function @loop_move(%dst : ptr, %src : ptr, %count : i32) -> ptr [unwind=no, binding=strong, object=cppgm_builtin_memmove]

function @observe() -> i32 [binding=strong] {
  slot $source : obj<8x4>
  slot $copy : obj<8x4>
  block ^entry:
    %src = addr $source
    %dst = addr $copy
    store typed i32 17, %src
    %a = call ptr @source_move(%dst, %src, 4)
    %b = call ptr @loop_move(%src, %dst, 4)
    %result = load typed i32 %src
    return i32 %result
}

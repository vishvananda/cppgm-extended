target wasm32 1

function @swap_loop() -> i32 {
  block ^entry:
    jump ^loop
  block ^loop:
    %a = phi i32 [^entry: 3, ^body: %b]
    %b = phi i32 [^entry: 7, ^body: %a]
    %n = phi i32 [^entry: 0, ^body: %next]
    %test = cmp lt i32 %n, 3
    branch %test, ^body, ^exit
  block ^body:
    %next = binary add i32 %n, 1
    jump ^loop
  block ^exit:
    %tens = binary mul i32 %a, 10
    %result = binary add i32 %tens, %b
    return i32 %result
}

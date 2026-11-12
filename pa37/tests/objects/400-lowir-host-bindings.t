target wasm32 1
import @from_host "host" "add"
import @from_empty "" ""
declare function @from_host(%x : i32) -> i32
declare function @from_empty(%x : i32) -> i32
function @dispatch(%x : i32) -> i32 [binding=internal] {
  block ^entry:
    %a = call i32 @from_host(%x)
    %b = call i32 @from_empty(%x)
    %result = binary add i32 %a, %b
    return i32 %result
}
export @dispatch "answer with spaces"
export @from_host "forwarded"
export @dispatch ""

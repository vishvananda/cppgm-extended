declare function @value(%x : i64) -> i64 [effects=readnone, repeatable=yes]
function @forward(%fn : ptr, %x : i64) -> i64 {
  block ^entry:
    %a = call i64 @value(%x)
    %b = call i64 %fn(%a) as (%v : i64) -> i64 [effects=readonly, repeatable=yes]
    return i64 %b
}

# Library math calls stay calls at -O1.

declare function @fabs(%x : f64) -> f64 [linkage=c, binding=strong, object=fabs]
declare function @pow(%x : f64, %y : f64) -> f64 [linkage=c, binding=strong, object=pow]

function @use(%x : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %a = call f64 @fabs(%x)
    %r = call f64 @pow(%a, 2.0)
    return f64 %r
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}

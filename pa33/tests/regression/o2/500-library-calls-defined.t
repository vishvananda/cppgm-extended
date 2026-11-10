# A unit's own definition of a library name keeps its call.

function @fabs(%x : f64) -> f64 [linkage=c, binding=strong, object=fabs, no_inline=yes] {
  block ^entry:
    %r = binary add f64 %x, 1.0
    return f64 %r
}

function @use(%x : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %r = call f64 @fabs(%x)
    return f64 %r
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}

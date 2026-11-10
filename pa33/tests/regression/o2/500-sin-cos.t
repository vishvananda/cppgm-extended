# `sin` and `cos` of one value become one `sincos`.

declare function @sin(%x : f64) -> f64 [linkage=c, binding=strong, object=sin]
declare function @cos(%x : f64) -> f64 [linkage=c, binding=strong, object=cos]
declare function @sinf(%x : f32) -> f32 [linkage=c, binding=strong, object=sinf]
declare function @cosf(%x : f32) -> f32 [linkage=c, binding=strong, object=cosf]

function @both(%x : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %s = call f64 @sin(%x)
    %c = call f64 @cos(%x)
    %r = binary div f64 %s, %c
    return f64 %r
}

# In either order, in the narrow type too, and the first call's block may be an
# earlier one that dominates the other.
function @apart(%x : f32, %k : i1) -> f32 [no_inline=yes] {
  block ^entry:
    %c = call f32 @cosf(%x)
    branch %k, ^then, ^done
  block ^then:
    %s = call f32 @sinf(%x)
    %m = binary mul f32 %s, %c
    return f32 %m
  block ^done:
    return f32 %c
}

# Other calls of that value read the same results.
function @three(%x : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %a = call f64 @sin(%x)
    %b = call f64 @cos(%x)
    %c = call f64 @sin(%x)
    %r = binary add f64 %a, %b
    %t = binary add f64 %r, %c
    return f64 %t
}

# Different values, and two calls of one function, stay as they are.
function @different(%x : f64, %y : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %s = call f64 @sin(%x)
    %c = call f64 @cos(%y)
    %r = binary add f64 %s, %c
    return f64 %r
}

function @same_function(%x : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %s = call f64 @sin(%x)
    %c = call f64 @sin(%x)
    %r = binary add f64 %s, %c
    return f64 %r
}

# Neither block dominates the other.
function @arms(%x : f64, %k : i1) -> f64 [no_inline=yes] {
  block ^entry:
    branch %k, ^t, ^f
  block ^t:
    %s = call f64 @sin(%x)
    return f64 %s
  block ^f:
    %c = call f64 @cos(%x)
    return f64 %c
}

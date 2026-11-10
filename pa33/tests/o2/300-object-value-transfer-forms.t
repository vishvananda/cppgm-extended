function @object_forms(%a : f64, %b : f64, %c : f64) -> f64 [no_inline=yes] {
  slot $source : obj<24x8>
  slot $target : obj<24x8>
  slot $last : obj<24x8>
  block ^entry:
    %p = addr $source
    %p8 = index i8 %p, 8
    %p16 = index i8 %p, 16
    store f64 %a, %p
    store f64 %b, %p8
    store f64 %c, %p16
    %snapshot = load obj<24x8> $source
    %alias = copy obj<24x8> %snapshot
    %another = copy obj<24x8> %alias
    store f64 99, %p
    copyobj 24x8 %another, $target
    %again = copy obj<24x8> $target
    store obj<24x8> %again, $last
    %q = addr $last
    %q8 = index i8 %q, 8
    %q16 = index i8 %q, 16
    %x = load f64 %q
    %y = load f64 %q8
    %z = load f64 %q16
    %xy = binary add f64 %x, %y
    %xyz = binary add f64 %xy, %z
    return f64 %xyz
}
function @main() -> i32 {
  block ^entry:
    %value = call f64 @object_forms(1.5, -2.25, 4.0)
    %bad = cmp ne f64 %value, 3.25
    %out = convert zext i32 i1 %bad
    return i32 %out
}

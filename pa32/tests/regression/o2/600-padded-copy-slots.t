# Two slots of a class with padding, one stored by its members and one read
# after a whole copy: the fields of both follow the narrow member, so both
# split. A copy in wider chunks would read narrow stores back.
function @sum(%a : ptr, %out : ptr) -> f32 [binding=strong] {
  slot $from : obj<8x4>
  slot $to : obj<8x4>

  block ^entry:
    %b = load i8 %a
    %f = index i8 [projection=field] %a, 4
    %v = load typed f32 %f
    %p = addr $from
    store i8 %b, %p
    %q = index i8 [projection=field] %p, 4
    store typed f32 %v, %q
    %r = addr $to
    copyobj 8x4 $from, %r
    %s = index i8 [projection=field] %r, 4
    %w = load typed f32 %s
    return f32 %w
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}

# Complex values move as whole objects from slot to slot: the loads whose
# values only stores read, and a call's result, split with the slots, so the
# fields reach the arithmetic without a copy through memory.
declare function @slow(%a : f64, %b : f64) -> obj<16x8> [unwind=no]

function @square_add(%re : f64, %im : f64, %c : f64, %retry : i1) -> f64 [binding=strong] {
  slot $z : obj<16x8>
  slot $product : obj<16x8>
  slot $result : obj<16x8>

  block ^entry:
    %z_re = addr $z
    store typed f64 %re, %z_re
    %z_im = index i8 [projection=field] %z_re, 8
    store typed f64 %im, %z_im
    %square = binary mul f64 %re, %re
    %whole = load obj<16x8> $z
    store obj<16x8> %whole, $product
    branch %retry, ^again, ^merge

  block ^again:
    %value = call obj<16x8> @slow(%re, %im)
    store obj<16x8> %value, $product
    jump ^merge

  block ^merge:
    %moved = load obj<16x8> $product
    store obj<16x8> %moved, $result
    %read_re = addr $result
    %first = load typed f64 %read_re
    %read_im = index i8 [projection=field] %read_re, 8
    %second = load typed f64 %read_im
    %sum = binary add f64 %first, %second
    %out = binary add f64 %sum, %c
    %total = binary add f64 %out, %square
    return f64 %total
}

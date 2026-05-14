function @main() -> i64 {
  slot $tmp : i64
  block ^entry:
    %base = addr $tmp
    %field = index i8 [projection=field] %base, 4
    %subobj = index i8 [projection=base_subobject] %field, 0
    %refslot = index i8 [projection=reference_field] %subobj, 8
    %elem = index i64 [projection=array_element] %base, 1
    %is_refslot = cmp ne ptr %refslot, 0
    %is_elem = cmp ne ptr %elem, 0
    %both = binary and i64 %is_refslot, %is_elem
    return i64 %both
}

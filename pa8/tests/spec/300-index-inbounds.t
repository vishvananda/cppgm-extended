function @element(%base : ptr, %offset : i64) -> ptr {
  block ^entry:
    %p = index inbounds i32 [projection=array_element] %base, %offset
    return ptr %p
}

function @plain(%base : ptr, %offset : i64) -> ptr {
  block ^entry:
    %p = index i32 [projection=array_element] %base, %offset
    return ptr %p
}

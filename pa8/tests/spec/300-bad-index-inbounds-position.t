function @invalid(%base : ptr) -> ptr {
  block ^entry:
    %p = index i32 inbounds %base, 1
    return ptr %p
}

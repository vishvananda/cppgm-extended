function @invalid(%base : ptr) -> ptr {
  block ^entry:
    %p = copy inbounds ptr %base
    return ptr %p
}

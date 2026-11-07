declare function @opaque [object=external_function]
global @address : ptr = addr @opaque
function @take() -> ptr {
  block ^entry:
    %p = addr @opaque
    return ptr %p
}

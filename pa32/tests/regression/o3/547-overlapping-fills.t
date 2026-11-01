# A loop of literal count that stores two runs becomes two fills only when the
# runs are in different objects: the stores of one pass overwrite each other
# otherwise, and the order of the passes decides the result.

global @table : obj<320x8> = { zero 320 }
global @other : obj<320x8> = { zero 320 }

function @same_object() -> void [no_inline=yes] {
  block ^entry:
    %second = index i64 @table, 1
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^body: %next]
    %c = cmp lt i64 %i, 30
    branch %c, ^body, ^done
  block ^body:
    %p = index i64 @table, %i
    store i64 1, %p
    %q = index i64 %second, %i
    store i64 2, %q
    %next = binary add i64 %i, 1
    jump ^head
  block ^done:
    return void
}

function @two_pointers(%a : ptr, %b : ptr) -> void [no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^body: %next]
    %c = cmp lt i64 %i, 30
    branch %c, ^body, ^done
  block ^body:
    %p = index i64 %a, %i
    store i64 1, %p
    %q = index i64 %b, %i
    store i64 2, %q
    %next = binary add i64 %i, 1
    jump ^head
  block ^done:
    return void
}

function @two_objects() -> void [no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^body: %next]
    %c = cmp lt i64 %i, 30
    branch %c, ^body, ^done
  block ^body:
    %p = index i64 @table, %i
    store i64 1, %p
    %q = index i64 @other, %i
    store i64 2, %q
    %next = binary add i64 %i, 1
    jump ^head
  block ^done:
    return void
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}

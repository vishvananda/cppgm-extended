# At -O2 a call through a vtable slot, passing the object first, where the
# object's construction (a store of a readonly table's address point into
# it) dominates the call, compares the loaded callee with the function the
# table holds in that slot and calls that function directly when they
# match; the late wave then inlines the direct call. In @other the object
# comes from the caller: no store dominates the call, and it stays as it
# is.
declare function @operatornew(%arg0 : i64) -> ptr [role=allocate_memory, binding=strong, object=cppgm_builtin_operator_new]

global @toggle_table [storage=readonly, binding=internal] = {
  i64 0
  i64 0
  ptr addr @flip
  ptr addr @tally
}

function @flip(%this : ptr) -> ptr [binding=internal] {
  block ^entry:
    %at = index i8 %this, 8
    %v = load u8 %at
    %w = binary xor u8 %v, 1
    store u8 %w, %at
    return ptr %this
}

function @tally(%this : ptr) -> void [binding=internal] {
  block ^entry:
    %at = index i8 %this, 12
    %v = load i32 %at
    %w = binary add i32 %v, 1
    store i32 %w, %at
    return void
}

function @other(%obj : ptr) -> void [binding=internal, no_inline=yes] {
  block ^entry:
    %vptr = load ptr %obj
    %f = load ptr %vptr
    %r = call ptr %f(%obj) as (%this : ptr) -> ptr
    return void
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %obj = call ptr @operatornew(16)
    %table = addr @toggle_table
    %point = index i8 %table, 16
    store ptr %point, %obj
    %state = index i8 %obj, 8
    store u8 1, %state
    %count = index i8 %obj, 12
    store i32 0, %count
    jump ^loop

  block ^loop:
    %i = phi i32 [^entry: 0, ^loop: %next]
    %vptr = load ptr %obj
    %f = load ptr %vptr
    %r = call ptr %f(%obj) as (%this : ptr) -> ptr
    %again = load ptr %obj
    %slot = index i8 %again, 8
    %g = load ptr %slot
    call void %g(%obj) as (%this : ptr) -> void
    %next = binary add i32 %i, 1
    %more = cmp lt i32 %next, 6
    branch %more, ^loop, ^done

  block ^done:
    call void @other(%obj)
    %s = load u8 %state
    %wide = convert zext i32 u8 %s
    %hundreds = binary mul i32 %wide, 100
    %n = load i32 %count
    %result = binary add i32 %hundreds, %n
    return i32 %result
}

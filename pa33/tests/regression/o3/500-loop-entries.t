# The test before this loop enters it from two blocks: the bound comes from a
# phi of the preheader the two now share. The loop then rotates, and the global
# it updates on each pass stays in a register.
global @state : i32 [binding=internal] = 42

function @capped(%n : i32, %p : ptr) -> i32 [no_inline=yes] {
  block ^entry:
    %big = cmp gt i32 %n, 60
    branch %big, ^cap, ^loop

  block ^cap:
    jump ^loop

  block ^loop:
    %limit = phi i32 [^entry: %n, ^cap: 60, ^body: %limit]
    %count = phi i32 [^entry: %n, ^cap: 60, ^body: %limit]
    %i = phi i32 [^entry: 0, ^cap: 0, ^body: %next]
    %more = cmp lt i32 %i, %count
    branch %more, ^body, ^done

  block ^body:
    %old = load typed i32 @state
    %new = binary add nowrap i32 %old, 3
    store typed i32 %new, @state
    %wide = convert sext i64 i32 %new
    %at = index i64 %p, %i
    store typed i64 %wide, %at
    %next = binary add nowrap i32 %i, 1
    jump ^loop

  block ^done:
    return i32 %limit
}

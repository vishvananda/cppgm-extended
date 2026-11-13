# A target loaded only for an indirect jump is read by the jump itself
# (jmp [m]), with no register load before it: `@pick` jumps through the
# table entry.
declare global @pick.first : i8 [binding=internal]
declare global @pick.second : i8 [binding=internal]

global @targets [binding=internal] = {
  ptr addr @pick.first
  ptr addr @pick.second
}

function @pick(%which : i64) -> i32 [binding=strong, no_inline=yes] {
  code_label @pick.first = ^first
  code_label @pick.second = ^second
  block ^entry:
    %table = addr @targets
    %entry = index ptr %table, %which
    %target = load ptr %entry
    indirect_jump %target, ^first, ^second

  block ^first:
    return i32 3

  block ^second:
    return i32 5
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call i32 @pick(0)
    %b = call i32 @pick(1)
    %t = binary add i32 %a, %b
    return i32 %t
}

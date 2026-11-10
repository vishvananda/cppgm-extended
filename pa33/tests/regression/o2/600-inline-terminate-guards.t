# A noexcept function runs its body inside a terminate guard: a catch-all
# landing that reads the exception and calls a function that neither returns
# nor unwinds. A callee whose only catches are such guards, and which cannot
# unwind, goes inline at -O2, also inside the caller's own `try` region: the
# copy's guard catches all that the copy raises. A callee that also catches a
# type stays a call, as does a guarded callee in handler code.
declare function @poke(%value : i64) -> i64
declare function @report(%e : ptr) -> void [unwind=no, return=noreturn]

global @bang_type : i8 = 0

function @checked(%value : i64) -> i64 [unwind=no] {
  block ^entry:
    eh_try ^guard
    %small = cmp lt i64 %value, 1000
    branch %small, ^fast, ^slow

  block ^fast:
    %r = binary mul i64 %value, 3
    eh_end
    return i64 %r

  block ^slow:
    %p = call i64 @poke(%value)
    eh_end
    return i64 %p

  block ^guard:
    eh_catch_all, 1
    %e = exception ptr
    call void @report(%e)
    unreachable
}

function @typed(%value : i64) -> i64 [unwind=no] {
  block ^entry:
    eh_try ^dispatch
    %p = call i64 @poke(%value)
    eh_end
    return i64 %p

  block ^dispatch:
    eh_catch @bang_type, 1
    eh_catch_all, 2
    %e = exception ptr
    call void @report(%e)
    unreachable
}

function @caller(%value : i64) -> i64 {
  block ^entry:
    %a = call i64 @checked(%value)
    eh_try ^dispatch
    %b = call i64 @checked(%a)
    %c = call i64 @typed(%b)
    eh_end
    %s = binary add i64 %a, %c
    return i64 %s

  block ^dispatch:
    eh_catch @bang_type, 1
    %selector = exception_selector i32
    %matched = cmp eq i32 %selector, 1
    branch %matched, ^caught, ^next

  block ^caught:
    %d = call i64 @checked(%value)
    return i64 %d

  block ^next:
    resume
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}

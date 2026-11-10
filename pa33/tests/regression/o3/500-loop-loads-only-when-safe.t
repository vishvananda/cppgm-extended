# A location read before the loop must not fault there. In @tally the loop
# reads and writes through %p on odd passes only, so %p's load stays in the
# loop: @main passes a null pointer with one pass, which never reads it. In
# @counts the loop reads the global @seen on every pass and writes it on
# some; a defined global may always be read, and its load leaves the loop.

global @seen : i32 = 0

function @tally(%p : ptr, %n : i32) -> i32 [no_inline=yes] {
  block ^entry:
    jump ^head

  block ^head:
    %i = phi i32 [^entry: 0, ^next: %j]
    %s = phi i32 [^entry: 0, ^next: %t]
    %odd = binary and i32 %i, 1
    branch %odd, ^read, ^next

  block ^read:
    %v = load i32 %p
    %w = binary add i32 %v, 1
    store i32 %w, %p
    %u = binary add i32 %s, %v
    jump ^next

  block ^next:
    %t = phi i32 [^head: %s, ^read: %u]
    %j = binary add i32 %i, 1
    %go = cmp lt i32 %j, %n
    branch %go, ^head, ^done

  block ^done:
    return i32 %t
}

function @counts(%n : i32) -> i32 [no_inline=yes] {
  block ^entry:
    jump ^head

  block ^head:
    %i = phi i32 [^entry: 0, ^next: %j]
    %c = phi i32 [^entry: 0, ^next: %e]
    %g = load i32 @seen
    %low = binary and i32 %i, 3
    %bump = cmp eq i32 %low, 0
    branch %bump, ^up, ^next

  block ^up:
    %d = binary add i32 %c, 1
    %h = binary add i32 %g, %i
    store i32 %h, @seen
    jump ^next

  block ^next:
    %e = phi i32 [^head: %c, ^up: %d]
    %j = binary add i32 %i, 1
    %go = cmp lt i32 %j, %n
    branch %go, ^head, ^done

  block ^done:
    return i32 %e
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call i32 @tally(0, 1)
    %b = call i32 @counts(10)
    %s = load i32 @seen
    %x = binary mul i32 %b, 10
    %y = binary add i32 %x, %s
    %z = binary add i32 %y, %a
    return i32 %z
}

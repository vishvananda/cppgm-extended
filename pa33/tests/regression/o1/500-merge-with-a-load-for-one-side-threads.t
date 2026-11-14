# A merge holds a flag phi, a load that only one successor reads, and a
# branch on the flag. The branch does not need the load, so each predecessor
# whose flag decides the other side jumps there, past the load. The merge is
# then entered from one block, and the load runs only on the path that reads
# it.
global @cell : i64 = 40
global @seen : i64 = 0

function @note(%v : i64) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %old = load i64 @seen
    %new = binary add i64 %old, %v
    store i64 %new, @seen
    return void
}

function @choose(%x : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %low = binary and i64 %x, 3
    %zero = cmp eq i64 %low, 0
    branch %zero, ^holds, ^other

  block ^holds:
    call void @note(1)
    jump ^merge

  block ^other:
    %one = cmp eq i64 %low, 1
    branch %one, ^first, ^second

  block ^first:
    call void @note(2)
    jump ^merge

  block ^second:
    call void @note(4)
    jump ^merge

  block ^merge:
    %flag = phi i64 [^holds: 1, ^first: 0, ^second: 0]
    %p = addr @cell
    %value = load i64 %p
    branch %flag, ^use, ^skip

  block ^use:
    %sum = binary add i64 %value, %x
    jump ^done

  block ^skip:
    jump ^done

  block ^done:
    %r = phi i64 [^use: %sum, ^skip: 7]
    return i64 %r
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call i64 @choose(8)
    %b = call i64 @choose(5)
    %c = call i64 @choose(6)
    %s1 = binary add i64 %a, %b
    %s2 = binary add i64 %s1, %c
    %seen = load i64 @seen
    %s3 = binary add i64 %s2, %seen
    %t = convert trunc i32 i64 %s3
    return i32 %t
}

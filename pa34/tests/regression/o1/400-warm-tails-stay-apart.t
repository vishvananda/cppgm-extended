# A tail of at most 16 instructions that the block before it runs into
# stays a copy of its own where it runs at least a quarter as often as the
# function's entry (placement's estimate): as a jump to its twin, that path
# would take a branch to save a few bytes. In @fill the two tails of six
# stores stay in line, one behind a sign test (3 times in 10) and one on its
# other side. In @rare the same tails sit behind a second sign test and run
# less than a quarter as often as the entry: the second path jumps to the
# first one's tail.
global @seen : i64 = 0
global @words : obj<48x8> = zero

function @fill(%k : i64, %v : i64, %p : ptr) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %neg = cmp lt i64 %k, 0
    branch %neg, ^low, ^high

  block ^low:
    %odd = binary and i64 %v, 1
    branch %odd, ^low_odd, ^low_tail

  block ^low_odd:
    store i64 %k, @seen
    jump ^low_tail

  block ^low_tail:
    store i64 %v, %p
    %l1 = index i8 %p, 8
    store i64 %v, %l1
    %l2 = index i8 %p, 16
    store i64 %v, %l2
    %l3 = index i8 %p, 24
    store i64 %v, %l3
    %l4 = index i8 %p, 32
    store i64 %v, %l4
    %l5 = index i8 %p, 40
    store i64 %v, %l5
    return void

  block ^high:
    %big = cmp gt i64 %k, 100
    branch %big, ^high_big, ^high_mid

  block ^high_big:
    store i64 %v, @seen
    jump ^high_tail

  block ^high_mid:
    %two = binary and i64 %k, 2
    branch %two, ^high_two, ^high_tail

  block ^high_two:
    store i64 %k, @seen
    jump ^high_tail

  block ^high_tail:
    store i64 %v, %p
    %h1 = index i8 %p, 8
    store i64 %v, %h1
    %h2 = index i8 %p, 16
    store i64 %v, %h2
    %h3 = index i8 %p, 24
    store i64 %v, %h3
    %h4 = index i8 %p, 32
    store i64 %v, %h4
    %h5 = index i8 %p, 40
    store i64 %v, %h5
    return void
}

function @rare(%k : i64, %m : i64, %v : i64, %p : ptr) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %neg = cmp lt i64 %k, 0
    branch %neg, ^low, ^skip

  block ^low:
    %neg2 = cmp lt i64 %m, 0
    branch %neg2, ^first, ^second

  block ^first:
    store i64 %k, @seen
    jump ^first_tail

  block ^first_tail:
    store i64 %v, %p
    %f1 = index i8 %p, 8
    store i64 %v, %f1
    %f2 = index i8 %p, 16
    store i64 %v, %f2
    %f3 = index i8 %p, 24
    store i64 %v, %f3
    %f4 = index i8 %p, 32
    store i64 %v, %f4
    %f5 = index i8 %p, 40
    store i64 %v, %f5
    return void

  block ^second:
    store i64 %m, @seen
    jump ^second_tail

  block ^second_tail:
    store i64 %v, %p
    %s1 = index i8 %p, 8
    store i64 %v, %s1
    %s2 = index i8 %p, 16
    store i64 %v, %s2
    %s3 = index i8 %p, 24
    store i64 %v, %s3
    %s4 = index i8 %p, 32
    store i64 %v, %s4
    %s5 = index i8 %p, 40
    store i64 %v, %s5
    return void

  block ^skip:
    %big = cmp gt i64 %v, 100
    branch %big, ^skip_big, ^done

  block ^skip_big:
    store i64 %v, @seen
    jump ^done

  block ^done:
    return void
}


function @main() -> i32 [role=entry] {
  block ^entry:
    call void @fill(-3, 5, @words)
    call void @fill(200, 6, @words)
    call void @fill(7, 9, @words)
    call void @rare(-1, -2, 11, @words)
    call void @rare(-1, 4, 12, @words)
    call void @rare(5, 4, 300, @words)
    %s = load i64 @seen
    %w = index i8 @words, 40
    %x = load i64 %w
    %t1 = binary add i64 %s, %x
    %t = convert trunc i32 i64 %t1
    return i32 %t
}

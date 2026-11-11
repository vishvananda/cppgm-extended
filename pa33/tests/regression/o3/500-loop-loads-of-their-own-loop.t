# A loop's locations come from its own accesses. In @count the second loop
# (^drain) reads and writes through %q, which the first loop leaves null
# until a pass on which @pick returns 1 allocates it. The first loop calls
# @pick, so only locations it loads itself may be read from a slot there:
# reading %q's location in its preheader or after the call would read
# through a null pointer on the first pass, and would decide the test of %q
# before the allocation. (The first loop leaves from two blocks, so the code
# after it reads the header's %q.)

declare function @malloc(%arg0 : i64) -> ptr [unwind=no, linkage=c, binding=strong]
declare function @free(%arg0 : ptr) -> void [unwind=no, linkage=c, binding=strong]

global @hits : i32 = 0

function @pick(%i : i32) -> i32 [no_inline=yes] {
  block ^entry:
    %h = load i32 @hits
    %h1 = binary add i32 %h, 1
    store i32 %h1, @hits
    %odd = binary and i32 %i, 1
    return i32 %odd
}

function @count(%n : i32) -> i32 [no_inline=yes] {
  block ^entry:
    jump ^head

  block ^head:
    %i = phi i32 [^entry: 0, ^next: %j]
    %q = phi ptr [^entry: 0, ^next: %r]
    %go = cmp lt i32 %i, %n
    branch %go, ^body, ^test

  block ^test:
    %h = load i32 @hits
    %late = cmp eq i32 %h, 1000
    branch %late, ^body, ^after

  block ^body:
    %k = call i32 @pick(%i)
    branch %k, ^add, ^next

  block ^add:
    %none = cmp eq ptr %q, 0
    branch %none, ^make, ^bump

  block ^make:
    %m = call ptr @malloc(8)
    store i32 0, %m
    jump ^bump

  block ^bump:
    %p = phi ptr [^add: %q, ^make: %m]
    %c = load i32 %p
    %c1 = binary add i32 %c, 1
    store i32 %c1, %p
    jump ^next

  block ^next:
    %r = phi ptr [^body: %q, ^bump: %p]
    %j = binary add i32 %i, 1
    jump ^head

  block ^after:
    branch %q, ^drain, ^zero

  block ^drain:
    %t = phi i32 [^after: 0, ^drain: %t1]
    %v = load i32 %q
    %v1 = binary sub i32 %v, 1
    store i32 %v1, %q
    %t1 = binary add i32 %t, 1
    %more = cmp gt i32 %v1, 0
    branch %more, ^drain, ^done

  block ^done:
    call void @free(%q)
    return i32 %t1

  block ^zero:
    return i32 0
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call i32 @count(10)
    %h = load i32 @hits
    %s = binary add i32 %a, %h
    return i32 %s
}

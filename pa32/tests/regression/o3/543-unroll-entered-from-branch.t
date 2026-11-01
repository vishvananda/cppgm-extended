# A counted loop entered from a branching block unrolls: it gets a preheader
# first. The body is straight-line, touches a frame slot and is too large for
# the unroller of branching bodies.
global @seen : obj<8x8> = { zero 8 }

function @keep(%p : ptr) -> void [no_inline=yes] {
  block ^entry:
    store ptr %p, @seen
    return void
}

function @entered_from_branch(%c : i32, %k : i64) -> i64 [no_inline=yes] {
  slot $s : obj<8x8>
  block ^entry:
    %p = addr $s
    call void @keep(%p)
    store i64 0, %p
    branch %c, ^header, ^exit
  block ^header:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi i64 [^entry: %k, ^body: %v35]
    %more = cmp lt i64 %i, 5
    branch %more, ^body, ^exit
  block ^body:
    store i64 %s, %p
    %v0 = binary mul i64 %s, %k
    %v1 = binary sub i64 %v0, %k
    %v2 = binary mul i64 %v1, %k
    %v3 = binary sub i64 %v2, %k
    %v4 = binary mul i64 %v3, %k
    %v5 = binary sub i64 %v4, %k
    %v6 = binary mul i64 %v5, %k
    %v7 = binary sub i64 %v6, %k
    %v8 = binary mul i64 %v7, %k
    %v9 = binary sub i64 %v8, %k
    %v10 = binary mul i64 %v9, %k
    %v11 = binary sub i64 %v10, %k
    %v12 = binary mul i64 %v11, %k
    %v13 = binary sub i64 %v12, %k
    %v14 = binary mul i64 %v13, %k
    %v15 = binary sub i64 %v14, %k
    %v16 = binary mul i64 %v15, %k
    %v17 = binary sub i64 %v16, %k
    %v18 = binary mul i64 %v17, %k
    %v19 = binary sub i64 %v18, %k
    %v20 = binary mul i64 %v19, %k
    %v21 = binary sub i64 %v20, %k
    %v22 = binary mul i64 %v21, %k
    %v23 = binary sub i64 %v22, %k
    %v24 = binary mul i64 %v23, %k
    %v25 = binary sub i64 %v24, %k
    %v26 = binary mul i64 %v25, %k
    %v27 = binary sub i64 %v26, %k
    %v28 = binary mul i64 %v27, %k
    %v29 = binary sub i64 %v28, %k
    %v30 = binary mul i64 %v29, %k
    %v31 = binary sub i64 %v30, %k
    %v32 = binary mul i64 %v31, %k
    %v33 = binary sub i64 %v32, %k
    %v34 = binary mul i64 %v33, %k
    %v35 = binary sub i64 %v34, %k
    %next = binary add i64 %i, 1
    jump ^header
  block ^exit:
    %r = phi i64 [^entry: 1, ^header: %s]
    %last = load i64 %p
    %sum = binary add i64 %r, %last
    return i64 %sum
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call i64 @entered_from_branch(1, 2)
    %b = call i64 @entered_from_branch(0, 2)
    %s = binary add i64 %a, %b
    %out = convert trunc i32 i64 %s
    return i32 %out
}

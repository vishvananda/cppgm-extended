# Full unrolling's budgets for a body that touches a frame slot: passes and
# cloned instructions, tighter for a body with a call.
global @seen : obj<8x8> = { zero 8 }

function @keep(%p : ptr) -> void [no_inline=yes] {
  block ^entry:
    store ptr %p, @seen
    return void
}

function @opaque(%v : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %r = binary add i64 %v, 3
    return i64 %r
}

function @eight_passes(%k : i64) -> i64 [no_inline=yes] {
  slot $s : obj<8x8>
  block ^entry:
    %p = addr $s
    call void @keep(%p)
    jump ^header
  block ^header:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi i64 [^entry: %k, ^body: %out]
    %more = cmp lt i64 %i, 8
    branch %more, ^body, ^exit
  block ^body:
    store i64 %s, %p
    %v0 = binary mul i64 %s, %k
    %v1 = binary sub i64 %v0, %k
    %v2 = binary mul i64 %v1, %k
    %out = binary add i64 %v2, 1
    %next = binary add i64 %i, 1
    jump ^header
  block ^exit:
    %last = load i64 %p
    %r = binary add i64 %s, %last
    return i64 %r
}

function @nine_passes(%k : i64) -> i64 [no_inline=yes] {
  slot $s : obj<8x8>
  block ^entry:
    %p = addr $s
    call void @keep(%p)
    jump ^header
  block ^header:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi i64 [^entry: %k, ^body: %out]
    %more = cmp lt i64 %i, 9
    branch %more, ^body, ^exit
  block ^body:
    store i64 %s, %p
    %v0 = binary mul i64 %s, %k
    %v1 = binary sub i64 %v0, %k
    %v2 = binary mul i64 %v1, %k
    %out = binary add i64 %v2, 1
    %next = binary add i64 %i, 1
    jump ^header
  block ^exit:
    %last = load i64 %p
    %r = binary add i64 %s, %last
    return i64 %r
}

function @five_passes_within_size(%k : i64) -> i64 [no_inline=yes] {
  slot $s : obj<8x8>
  block ^entry:
    %p = addr $s
    call void @keep(%p)
    jump ^header
  block ^header:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi i64 [^entry: %k, ^body: %out]
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
    %v36 = binary mul i64 %v35, %k
    %v37 = binary sub i64 %v36, %k
    %v38 = binary mul i64 %v37, %k
    %v39 = binary sub i64 %v38, %k
    %v40 = binary mul i64 %v39, %k
    %v41 = binary sub i64 %v40, %k
    %v42 = binary mul i64 %v41, %k
    %v43 = binary sub i64 %v42, %k
    %v44 = binary mul i64 %v43, %k
    %v45 = binary sub i64 %v44, %k
    %v46 = binary mul i64 %v45, %k
    %v47 = binary sub i64 %v46, %k
    %v48 = binary mul i64 %v47, %k
    %v49 = binary sub i64 %v48, %k
    %v50 = binary mul i64 %v49, %k
    %v51 = binary sub i64 %v50, %k
    %v52 = binary mul i64 %v51, %k
    %v53 = binary sub i64 %v52, %k
    %v54 = binary mul i64 %v53, %k
    %v55 = binary sub i64 %v54, %k
    %v56 = binary mul i64 %v55, %k
    %v57 = binary sub i64 %v56, %k
    %v58 = binary mul i64 %v57, %k
    %v59 = binary sub i64 %v58, %k
    %v60 = binary mul i64 %v59, %k
    %v61 = binary sub i64 %v60, %k
    %v62 = binary mul i64 %v61, %k
    %v63 = binary sub i64 %v62, %k
    %v64 = binary mul i64 %v63, %k
    %v65 = binary sub i64 %v64, %k
    %v66 = binary mul i64 %v65, %k
    %v67 = binary sub i64 %v66, %k
    %v68 = binary mul i64 %v67, %k
    %v69 = binary sub i64 %v68, %k
    %v70 = binary mul i64 %v69, %k
    %v71 = binary sub i64 %v70, %k
    %v72 = binary mul i64 %v71, %k
    %v73 = binary sub i64 %v72, %k
    %v74 = binary mul i64 %v73, %k
    %v75 = binary sub i64 %v74, %k
    %v76 = binary mul i64 %v75, %k
    %v77 = binary sub i64 %v76, %k
    %v78 = binary mul i64 %v77, %k
    %v79 = binary sub i64 %v78, %k
    %v80 = binary mul i64 %v79, %k
    %out = binary add i64 %v80, 1
    %next = binary add i64 %i, 1
    jump ^header
  block ^exit:
    %last = load i64 %p
    %r = binary add i64 %s, %last
    return i64 %r
}

function @five_passes_over_size(%k : i64) -> i64 [no_inline=yes] {
  slot $s : obj<8x8>
  block ^entry:
    %p = addr $s
    call void @keep(%p)
    jump ^header
  block ^header:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi i64 [^entry: %k, ^body: %out]
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
    %v36 = binary mul i64 %v35, %k
    %v37 = binary sub i64 %v36, %k
    %v38 = binary mul i64 %v37, %k
    %v39 = binary sub i64 %v38, %k
    %v40 = binary mul i64 %v39, %k
    %v41 = binary sub i64 %v40, %k
    %v42 = binary mul i64 %v41, %k
    %v43 = binary sub i64 %v42, %k
    %v44 = binary mul i64 %v43, %k
    %v45 = binary sub i64 %v44, %k
    %v46 = binary mul i64 %v45, %k
    %v47 = binary sub i64 %v46, %k
    %v48 = binary mul i64 %v47, %k
    %v49 = binary sub i64 %v48, %k
    %v50 = binary mul i64 %v49, %k
    %v51 = binary sub i64 %v50, %k
    %v52 = binary mul i64 %v51, %k
    %v53 = binary sub i64 %v52, %k
    %v54 = binary mul i64 %v53, %k
    %v55 = binary sub i64 %v54, %k
    %v56 = binary mul i64 %v55, %k
    %v57 = binary sub i64 %v56, %k
    %v58 = binary mul i64 %v57, %k
    %v59 = binary sub i64 %v58, %k
    %v60 = binary mul i64 %v59, %k
    %v61 = binary sub i64 %v60, %k
    %v62 = binary mul i64 %v61, %k
    %v63 = binary sub i64 %v62, %k
    %v64 = binary mul i64 %v63, %k
    %v65 = binary sub i64 %v64, %k
    %v66 = binary mul i64 %v65, %k
    %v67 = binary sub i64 %v66, %k
    %v68 = binary mul i64 %v67, %k
    %v69 = binary sub i64 %v68, %k
    %v70 = binary mul i64 %v69, %k
    %v71 = binary sub i64 %v70, %k
    %v72 = binary mul i64 %v71, %k
    %v73 = binary sub i64 %v72, %k
    %v74 = binary mul i64 %v73, %k
    %v75 = binary sub i64 %v74, %k
    %v76 = binary mul i64 %v75, %k
    %v77 = binary sub i64 %v76, %k
    %v78 = binary mul i64 %v77, %k
    %v79 = binary sub i64 %v78, %k
    %v80 = binary mul i64 %v79, %k
    %v81 = binary sub i64 %v80, %k
    %out = binary add i64 %v81, 1
    %next = binary add i64 %i, 1
    jump ^header
  block ^exit:
    %last = load i64 %p
    %r = binary add i64 %s, %last
    return i64 %r
}

function @four_passes_with_call(%k : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^header
  block ^header:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi i64 [^entry: %k, ^body: %out]
    %more = cmp lt i64 %i, 4
    branch %more, ^body, ^exit
  block ^body:
    %out = call i64 @opaque(%s)
    %next = binary add i64 %i, 1
    jump ^header
  block ^exit:
    return i64 %s
}

function @five_passes_with_call(%k : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^header
  block ^header:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi i64 [^entry: %k, ^body: %out]
    %more = cmp lt i64 %i, 5
    branch %more, ^body, ^exit
  block ^body:
    %out = call i64 @opaque(%s)
    %next = binary add i64 %i, 1
    jump ^header
  block ^exit:
    return i64 %s
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call i64 @eight_passes(1)
    %b = call i64 @nine_passes(1)
    %c = call i64 @five_passes_within_size(1)
    %d = call i64 @five_passes_over_size(1)
    %e = call i64 @four_passes_with_call(1)
    %f = call i64 @five_passes_with_call(1)
    %s1 = binary add i64 %a, %b
    %s2 = binary add i64 %s1, %c
    %s3 = binary add i64 %s2, %d
    %s4 = binary add i64 %s3, %e
    %s5 = binary add i64 %s4, %f
    %out = convert trunc i32 i64 %s5
    return i32 %out
}

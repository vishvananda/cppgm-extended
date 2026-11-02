# Past the unit's growth budget a call to a trivial leaf still goes inline: its
# copy takes no more room than the call. @work's size divides the budget, which
# its copies spend to the last instruction (the unused copies then vanish), so
# the last callers keep their calls and only @last's free leaf still fits.
function @leaf(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    return i64 %x
}

function @work(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %v0 = binary add i64 %x, %x
    %v1 = binary xor i64 %v0, %x
    %v2 = binary mul i64 %v1, %x
    %v3 = binary add i64 %v2, %x
    %v4 = binary xor i64 %v3, %x
    %v5 = binary mul i64 %v4, %x
    %v6 = binary add i64 %v5, %x
    %v7 = binary xor i64 %v6, %x
    %v8 = binary mul i64 %v7, %x
    %v9 = binary add i64 %v8, %x
    %v10 = binary xor i64 %v9, %x
    %v11 = binary mul i64 %v10, %x
    %v12 = binary add i64 %v11, %x
    %v13 = binary xor i64 %v12, %x
    %v14 = binary mul i64 %v13, %x
    %v15 = binary add i64 %v14, %x
    %v16 = binary xor i64 %v15, %x
    %v17 = binary mul i64 %v16, %x
    %v18 = binary add i64 %v17, %x
    %v19 = binary xor i64 %v18, %x
    %v20 = binary mul i64 %v19, %x
    %v21 = binary add i64 %v20, %x
    %v22 = binary xor i64 %v21, %x
    %v23 = binary mul i64 %v22, %x
    %v24 = binary add i64 %v23, %x
    %v25 = binary xor i64 %v24, %x
    %v26 = binary mul i64 %v25, %x
    %v27 = binary add i64 %v26, %x
    %v28 = binary xor i64 %v27, %x
    %v29 = binary mul i64 %v28, %x
    %v30 = binary add i64 %v29, %x
    %v31 = binary xor i64 %v30, %x
    %v32 = binary mul i64 %v31, %x
    %v33 = binary add i64 %v32, %x
    %v34 = binary xor i64 %v33, %x
    %v35 = binary mul i64 %v34, %x
    %v36 = binary add i64 %v35, %x
    %v37 = binary xor i64 %v36, %x
    %v38 = binary mul i64 %v37, %x
    return i64 %v38
}

function @caller0(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller1(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller2(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller3(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller4(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller5(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller6(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller7(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller8(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller9(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller10(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller11(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller12(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller13(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller14(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller15(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller16(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller17(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller18(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller19(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller20(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller21(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller22(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller23(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller24(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller25(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller26(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller27(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller28(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller29(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller30(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller31(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller32(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller33(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller34(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller35(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller36(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller37(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller38(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller39(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller40(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller41(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller42(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller43(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller44(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller45(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller46(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller47(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller48(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller49(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller50(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller51(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller52(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller53(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller54(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller55(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller56(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller57(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller58(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller59(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller60(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller61(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller62(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller63(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller64(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller65(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller66(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller67(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller68(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller69(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller70(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller71(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller72(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller73(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller74(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller75(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller76(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller77(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller78(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @caller79(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r0 = call i64 @work(%x)
    %r1 = call i64 @work(%x)
    %r2 = call i64 @work(%x)
    %r3 = call i64 @work(%x)
    %r4 = call i64 @work(%x)
    %r5 = call i64 @work(%x)
    %r6 = call i64 @work(%x)
    %r7 = call i64 @work(%x)
    return i64 0
}

function @last(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %r = call i64 @leaf(%x)
    return i64 %r
}

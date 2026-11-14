# An unswitched loop is copied without what only the other path of its test
# reaches: the loop is sized by what the two copies keep together.

declare function @touch(%v : i64) -> void [linkage=c, binding=strong, object=touch, unwind=no]

# One path is long and the other is a call: the copies share the header and
# the latch.
function @long_path(%flag : i64, %p : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^header

  block ^header:
    %i = phi i64 [^entry: 0, ^latch: %next]
    %more = cmp lt i64 %i, %n
    branch %more, ^body, ^exit

  block ^body:
    %at = index i64 %p, %i
    %v = load typed i64 %at
    branch %flag, ^long, ^short

  block ^long:
    %a0 = binary mul i64 %v, 3
    %a1 = binary xor i64 %a0, %i
    %a2 = binary add i64 %a1, %v
    %a3 = binary ushr i64 %a2, 7
    %a4 = binary mul i64 %a3, 3
    %a5 = binary xor i64 %a4, %i
    %a6 = binary add i64 %a5, %v
    %a7 = binary ushr i64 %a6, 7
    %a8 = binary mul i64 %a7, 3
    %a9 = binary xor i64 %a8, %i
    %a10 = binary add i64 %a9, %v
    %a11 = binary ushr i64 %a10, 7
    %a12 = binary mul i64 %a11, 3
    %a13 = binary xor i64 %a12, %i
    %a14 = binary add i64 %a13, %v
    %a15 = binary ushr i64 %a14, 7
    %a16 = binary mul i64 %a15, 3
    %a17 = binary xor i64 %a16, %i
    %a18 = binary add i64 %a17, %v
    %a19 = binary ushr i64 %a18, 7
    %a20 = binary mul i64 %a19, 3
    %a21 = binary xor i64 %a20, %i
    %a22 = binary add i64 %a21, %v
    %a23 = binary ushr i64 %a22, 7
    %a24 = binary mul i64 %a23, 3
    %a25 = binary xor i64 %a24, %i
    %a26 = binary add i64 %a25, %v
    %a27 = binary ushr i64 %a26, 7
    %a28 = binary mul i64 %a27, 3
    %a29 = binary xor i64 %a28, %i
    %a30 = binary add i64 %a29, %v
    %a31 = binary ushr i64 %a30, 7
    %a32 = binary mul i64 %a31, 3
    %a33 = binary xor i64 %a32, %i
    %a34 = binary add i64 %a33, %v
    %a35 = binary ushr i64 %a34, 7
    %a36 = binary mul i64 %a35, 3
    %a37 = binary xor i64 %a36, %i
    %a38 = binary add i64 %a37, %v
    %a39 = binary ushr i64 %a38, 7
    %a40 = binary mul i64 %a39, 3
    %a41 = binary xor i64 %a40, %i
    %a42 = binary add i64 %a41, %v
    %a43 = binary ushr i64 %a42, 7
    %a44 = binary mul i64 %a43, 3
    %a45 = binary xor i64 %a44, %i
    %a46 = binary add i64 %a45, %v
    %a47 = binary ushr i64 %a46, 7
    %a48 = binary mul i64 %a47, 3
    %a49 = binary xor i64 %a48, %i
    %a50 = binary add i64 %a49, %v
    %a51 = binary ushr i64 %a50, 7
    %a52 = binary mul i64 %a51, 3
    %a53 = binary xor i64 %a52, %i
    %a54 = binary add i64 %a53, %v
    %a55 = binary ushr i64 %a54, 7
    %a56 = binary mul i64 %a55, 3
    %a57 = binary xor i64 %a56, %i
    %a58 = binary add i64 %a57, %v
    %a59 = binary ushr i64 %a58, 7
    %a60 = binary mul i64 %a59, 3
    %a61 = binary xor i64 %a60, %i
    %a62 = binary add i64 %a61, %v
    %a63 = binary ushr i64 %a62, 7
    %a64 = binary mul i64 %a63, 3
    %a65 = binary xor i64 %a64, %i
    %a66 = binary add i64 %a65, %v
    %a67 = binary ushr i64 %a66, 7
    %a68 = binary mul i64 %a67, 3
    %a69 = binary xor i64 %a68, %i
    %a70 = binary add i64 %a69, %v
    %a71 = binary ushr i64 %a70, 7
    store typed i64 %a71, %at
    jump ^latch

  block ^short:
    call void @touch(%v)
    jump ^latch

  block ^latch:
    %next = binary add i64 %i, 1
    jump ^header

  block ^exit:
    return void
}

# Both paths meet in a long block: each copy would hold it, and the loop
# stays.
function @long_join(%flag : i64, %p : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^header

  block ^header:
    %i = phi i64 [^entry: 0, ^latch: %next]
    %more = cmp lt i64 %i, %n
    branch %more, ^body, ^exit

  block ^body:
    %at = index i64 %p, %i
    %v = load typed i64 %at
    branch %flag, ^left, ^right

  block ^left:
    call void @touch(%v)
    jump ^join

  block ^right:
    call void @touch(%i)
    jump ^join

  block ^join:
    %b0 = binary mul i64 %v, 3
    %b1 = binary xor i64 %b0, %i
    %b2 = binary add i64 %b1, %v
    %b3 = binary ushr i64 %b2, 7
    %b4 = binary mul i64 %b3, 3
    %b5 = binary xor i64 %b4, %i
    %b6 = binary add i64 %b5, %v
    %b7 = binary ushr i64 %b6, 7
    %b8 = binary mul i64 %b7, 3
    %b9 = binary xor i64 %b8, %i
    %b10 = binary add i64 %b9, %v
    %b11 = binary ushr i64 %b10, 7
    %b12 = binary mul i64 %b11, 3
    %b13 = binary xor i64 %b12, %i
    %b14 = binary add i64 %b13, %v
    %b15 = binary ushr i64 %b14, 7
    %b16 = binary mul i64 %b15, 3
    %b17 = binary xor i64 %b16, %i
    %b18 = binary add i64 %b17, %v
    %b19 = binary ushr i64 %b18, 7
    %b20 = binary mul i64 %b19, 3
    %b21 = binary xor i64 %b20, %i
    %b22 = binary add i64 %b21, %v
    %b23 = binary ushr i64 %b22, 7
    %b24 = binary mul i64 %b23, 3
    %b25 = binary xor i64 %b24, %i
    %b26 = binary add i64 %b25, %v
    %b27 = binary ushr i64 %b26, 7
    %b28 = binary mul i64 %b27, 3
    %b29 = binary xor i64 %b28, %i
    %b30 = binary add i64 %b29, %v
    %b31 = binary ushr i64 %b30, 7
    %b32 = binary mul i64 %b31, 3
    %b33 = binary xor i64 %b32, %i
    %b34 = binary add i64 %b33, %v
    %b35 = binary ushr i64 %b34, 7
    %b36 = binary mul i64 %b35, 3
    %b37 = binary xor i64 %b36, %i
    %b38 = binary add i64 %b37, %v
    %b39 = binary ushr i64 %b38, 7
    %b40 = binary mul i64 %b39, 3
    %b41 = binary xor i64 %b40, %i
    %b42 = binary add i64 %b41, %v
    %b43 = binary ushr i64 %b42, 7
    %b44 = binary mul i64 %b43, 3
    %b45 = binary xor i64 %b44, %i
    %b46 = binary add i64 %b45, %v
    %b47 = binary ushr i64 %b46, 7
    %b48 = binary mul i64 %b47, 3
    %b49 = binary xor i64 %b48, %i
    %b50 = binary add i64 %b49, %v
    %b51 = binary ushr i64 %b50, 7
    %b52 = binary mul i64 %b51, 3
    %b53 = binary xor i64 %b52, %i
    %b54 = binary add i64 %b53, %v
    %b55 = binary ushr i64 %b54, 7
    %b56 = binary mul i64 %b55, 3
    %b57 = binary xor i64 %b56, %i
    %b58 = binary add i64 %b57, %v
    %b59 = binary ushr i64 %b58, 7
    %b60 = binary mul i64 %b59, 3
    %b61 = binary xor i64 %b60, %i
    %b62 = binary add i64 %b61, %v
    %b63 = binary ushr i64 %b62, 7
    %b64 = binary mul i64 %b63, 3
    %b65 = binary xor i64 %b64, %i
    %b66 = binary add i64 %b65, %v
    %b67 = binary ushr i64 %b66, 7
    %b68 = binary mul i64 %b67, 3
    %b69 = binary xor i64 %b68, %i
    %b70 = binary add i64 %b69, %v
    %b71 = binary ushr i64 %b70, 7
    store typed i64 %b71, %at
    jump ^latch

  block ^latch:
    %next = binary add i64 %i, 1
    jump ^header

  block ^exit:
    return void
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}

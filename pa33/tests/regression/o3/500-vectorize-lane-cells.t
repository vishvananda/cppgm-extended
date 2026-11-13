# An element of a local array that the pass picks by its count modulo K, reads
# and writes back, is a lane cell: K passes in a row take K different elements,
# so the lanes of a wide pass hold different elements, each in its passes'
# order. The elements stay in registers of lanes for the whole vector loop,
# which runs one copy of the lanes' code per register each round. They are
# loaded before the loop and stored when it stops; the scalar loop then goes
# on through memory.

# LangArena's buffer hash: eight u32 hashes, one byte each pass. Two registers
# of four lanes, two copies a round of eight passes.
function @digest(%data : ptr, %n : i64, %init : ptr, %out : ptr) -> void [no_inline=yes] {
  slot $hashes : obj<32x4>

  block ^entry:
    %h = addr $hashes
    copyobj 32x4 %init, %h
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %r = binary umod i64 %i, 8
    %k = convert trunc u32 i64 %r
    %e = index u32 %h, %k
    %old = load typed u32 %e
    %s5 = binary shl u32 %old, 5
    %a = binary add u32 %old, %s5
    %p = index u8 %data, %i
    %b = load u8 %p
    %z = convert zext u32 u8 %b
    %c = binary add u32 %a, %z
    %s10 = binary shl u32 %c, 10
    %d = binary add u32 %c, %s10
    %u6 = binary ushr u32 %c, 6
    %new = binary xor u32 %d, %u6
    store typed u32 %new, %e
    %next = binary add i64 %i, 1
    %more = cmp ult i64 %next, %n
    branch %more, ^body, ^done
  block ^done:
    copyobj 32x4 %h, %out
    return void
}

# A mask is a remainder too. Four i32 elements take one register: one copy a
# round of four passes.
function @mask(%x : ptr, %n : i64, %out : ptr) -> void [no_inline=yes] {
  slot $sums : obj<16x4>

  block ^entry:
    %s = addr $sums
    copyobj 16x4 %out, %s
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %r = binary and i64 %i, 3
    %e = index i32 %s, %r
    %old = load typed i32 %e
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %twice = binary add i32 %old, %old
    %new = binary xor i32 %twice, %v
    store typed i32 %new, %e
    %next = binary add i64 %i, 1
    %more = cmp ult i64 %next, %n
    branch %more, ^body, ^done
  block ^done:
    copyobj 16x4 %s, %out
    return void
}

# Floating elements keep their passes' order, lane by lane: no sum is
# reassociated. A signed remainder of a 32-bit counter that counts up from
# zero is the same remainder.
function @decay(%x : ptr, %w : f32, %n : i32, %out : ptr) -> void [no_inline=yes] {
  slot $acc : obj<16x4>

  block ^entry:
    %s = addr $acc
    copyobj 16x4 %out, %s
    jump ^body
  block ^body:
    %i = phi i32 [^entry: 0, ^body: %next]
    %r = binary mod i32 %i, 4
    %k = convert sext i64 i32 %r
    %e = index f32 %s, %k
    %old = load typed f32 %e
    %wide = convert sext i64 i32 %i
    %p = index f32 %x, %wide
    %v = load typed f32 %p
    %m = binary mul f32 %old, %w
    %new = binary add f32 %m, %v
    store typed f32 %new, %e
    %next = binary add nowrap i32 %i, 1
    %more = cmp lt i32 %next, %n
    branch %more, ^body, ^done
  block ^done:
    copyobj 16x4 %s, %out
    return void
}

# The count starts at 4, so the first pass takes the second register: each
# round's first copy writes it, its second copy the first.
function @offset(%data : ptr, %n : i64, %out : ptr) -> void [no_inline=yes] {
  slot $hashes : obj<32x4>

  block ^entry:
    %h = addr $hashes
    copyobj 32x4 %out, %h
    %more0 = cmp ult i64 4, %n
    branch %more0, ^body, ^done
  block ^body:
    %i = phi i64 [^entry: 4, ^body: %next]
    %r = binary umod i64 %i, 8
    %e = index u32 %h, %r
    %old = load typed u32 %e
    %p = index u8 %data, %i
    %b = load u8 %p
    %z = convert zext u32 u8 %b
    %new = binary add u32 %old, %z
    store typed u32 %new, %e
    %next = binary add i64 %i, 1
    %more = cmp ult i64 %next, %n
    branch %more, ^body, ^done
  block ^done:
    copyobj 32x4 %h, %out
    return void
}

declare function @observe(%p : ptr) -> void [unwind=no]

# The array's address reaches a call: another pointer may reach the array, so
# the loop stays scalar.
function @escaped(%data : ptr, %n : i64, %out : ptr) -> void [no_inline=yes] {
  slot $hashes : obj<32x4>

  block ^entry:
    %h = addr $hashes
    copyobj 32x4 %out, %h
    call void @observe(%h)
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %r = binary umod i64 %i, 8
    %e = index u32 %h, %r
    %old = load typed u32 %e
    %p = index u8 %data, %i
    %b = load u8 %p
    %z = convert zext u32 u8 %b
    %new = binary add u32 %old, %z
    store typed u32 %new, %e
    %next = binary add i64 %i, 1
    %more = cmp ult i64 %next, %n
    branch %more, ^body, ^done
  block ^done:
    copyobj 32x4 %h, %out
    return void
}

# The pass also reads the array's first element: scalar.
function @also_read(%data : ptr, %n : i64, %out : ptr) -> void [no_inline=yes] {
  slot $hashes : obj<32x4>

  block ^entry:
    %h = addr $hashes
    copyobj 32x4 %out, %h
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %r = binary umod i64 %i, 8
    %e = index u32 %h, %r
    %old = load typed u32 %e
    %first = load typed u32 %h
    %p = index u8 %data, %i
    %b = load u8 %p
    %z = convert zext u32 u8 %b
    %sum = binary add u32 %old, %z
    %new = binary xor u32 %sum, %first
    store typed u32 %new, %e
    %next = binary add i64 %i, 1
    %more = cmp ult i64 %next, %n
    branch %more, ^body, ^done
  block ^done:
    copyobj 32x4 %h, %out
    return void
}

# The count starts at 1: the first pass's element is in no register's first
# lane. Scalar.
function @misaligned(%data : ptr, %n : i64, %out : ptr) -> void [no_inline=yes] {
  slot $hashes : obj<32x4>

  block ^entry:
    %h = addr $hashes
    copyobj 32x4 %out, %h
    %more0 = cmp ult i64 1, %n
    branch %more0, ^body, ^done
  block ^body:
    %i = phi i64 [^entry: 1, ^body: %next]
    %r = binary umod i64 %i, 8
    %e = index u32 %h, %r
    %old = load typed u32 %e
    %p = index u8 %data, %i
    %b = load u8 %p
    %z = convert zext u32 u8 %b
    %new = binary add u32 %old, %z
    store typed u32 %new, %e
    %next = binary add i64 %i, 1
    %more = cmp ult i64 %next, %n
    branch %more, ^body, ^done
  block ^done:
    copyobj 32x4 %h, %out
    return void
}

# Two u32 elements fill no register of four lanes: scalar.
function @pair(%data : ptr, %n : i64, %out : ptr) -> void [no_inline=yes] {
  slot $hashes : obj<8x4>

  block ^entry:
    %h = addr $hashes
    copyobj 8x4 %out, %h
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %r = binary umod i64 %i, 2
    %e = index u32 %h, %r
    %old = load typed u32 %e
    %p = index u8 %data, %i
    %b = load u8 %p
    %z = convert zext u32 u8 %b
    %new = binary add u32 %old, %z
    store typed u32 %new, %e
    %next = binary add i64 %i, 1
    %more = cmp ult i64 %next, %n
    branch %more, ^body, ^done
  block ^done:
    copyobj 8x4 %h, %out
    return void
}

function @overwrite_word(%base : ptr, %i : i64, %seed : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %p = index i64 %base, %i
    %first = binary add i64 %seed, 11
    store i64 %first, %p
    %last = binary xor i64 %first, 99
    store i64 %last, %p
    return i64 %last
}

function @overwrite_indexed(%base : ptr, %i : i64, %seed : u32) -> u32 [binding=strong, no_inline=yes] {
  block ^entry:
    %p = index u32 %base, %i
    store u32 %seed, %p
    %shift = binary shl u32 %seed, 5
    %last = binary xor u32 %shift, %seed
    store u32 %last, %p
    return u32 %last
}

function @observed(%p : ptr, %q : ptr, %value : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    store i64 %value, %p
    %seen = load i64 %q
    %last = binary add i64 %value, 19
    store i64 %last, %p
    return i64 %seen
}

function @volatile_read(%p : ptr, %q : ptr, %value : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    store i64 %value, %p
    %seen = load volatile i64 %q
    %last = binary add i64 %value, 19
    store i64 %last, %p
    return i64 %seen
}

function @partial_write(%p : ptr, %value : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    store i64 %value, %p
    store u8 60, %p
    %out = load i64 %p
    return i64 %out
}

function @cross_block(%p : ptr, %flag : i64, %value : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    store i64 %value, %p
    branch %flag, ^write, ^exit
  block ^write:
    %last = binary add i64 %value, 19
    store i64 %last, %p
    jump ^exit
  block ^exit:
    %out = load i64 %p
    return i64 %out
}

function @volatile_writes(%p : ptr, %value : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    store volatile i64 %value, %p
    %last = binary add i64 %value, 19
    store volatile i64 %last, %p
    %out = load i64 %p
    return i64 %out
}

function @overwrite_float(%p : ptr, %first : f64, %last : f64) -> f64 [binding=strong, no_inline=yes] {
  block ^entry:
    store f64 %first, %p
    store f64 %last, %p
    %out = load f64 %p
    return f64 %out
}

function @absolute_address(%value : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    store i64 %value, 268435456
    %last = binary xor i64 %value, 99
    store i64 %last, 268435456
    return i64 %last
}

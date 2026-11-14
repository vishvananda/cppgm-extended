# A select's arm holds two instructions; a copy only renames a value and is
# not counted. `@both_fields` tests two bit-fields of one word (`id == 12 &&
# sub == bw`): its second block reads the field through a shift, a mask and a
# copy until the field's read is narrowed to the logical shift alone, and
# the select forms after that. `@renamed` has two instructions and two
# copies in its arm.
global @word_storage = {
  zero 16
}

function @both_fields(%p : ptr, %bw : u32) -> u8 [binding=strong, no_inline=yes] {
  block ^entry:
    %at = index inbounds i8 %p, 8
    %w = load i32 %at
    %low = binary and i32 %w, 255
    %is = cmp eq i32 %low, 12
    branch %is, ^second, ^done

  block ^second:
    %w2 = load i32 %at
    %high = binary shr i32 %w2, 8
    %field = binary and i32 %high, 16777215
    %u = copy u32 %field
    %same = cmp eq u32 %u, %bw
    jump ^done

  block ^done:
    %r = phi i64 [^second: %same, ^entry: 0]
    %out = convert trunc u8 i64 %r
    return u8 %out
}

function @renamed(%x : i32, %y : i32) -> i32 [binding=strong, no_inline=yes] {
  block ^entry:
    %big = cmp gt i32 %x, 100
    branch %big, ^work, ^done

  block ^work:
    %ux = copy u32 %x
    %m = binary mul u32 %ux, 3
    %sm = copy i32 %m
    %s = binary add i32 %sm, %y
    jump ^done

  block ^done:
    %r = phi i32 [^work: %s, ^entry: %y]
    return i32 %r
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %at = index inbounds i8 @word_storage, 8
    store i32 8204, %at
    %a = call u8 @both_fields(@word_storage, 32)
    %b = call u8 @both_fields(@word_storage, 33)
    store i32 8205, %at
    %c = call u8 @both_fields(@word_storage, 32)
    %d = call i32 @renamed(200, 7)
    %e = call i32 @renamed(2, 7)
    %ok0 = cmp eq u8 %a, 1
    %ok1 = cmp eq u8 %b, 0
    %ok2 = cmp eq u8 %c, 0
    %ok3 = cmp eq i32 %d, 607
    %ok4 = cmp eq i32 %e, 7
    %a0 = binary and i64 %ok0, %ok1
    %a1 = binary and i64 %a0, %ok2
    %a2 = binary and i64 %a1, %ok3
    %a3 = binary and i64 %a2, %ok4
    %r = convert trunc i32 i64 %a3
    %code = binary sub i32 1, %r
    return i32 %code
}

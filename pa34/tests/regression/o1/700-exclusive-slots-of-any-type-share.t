# Slots of different types that no execution uses both of take one storage:
# the three arms of @pick name one slot each, so its frame holds the largest
# of them and not their sum. $kept is named in the entry block, which reaches
# every arm, and keeps storage of its own.
function @ends(%base : ptr, %last : i64) -> i64 [unwind=no] {
  block ^entry:
    %first = load i64 %base
    %at = index i64 [projection=array_element] %base, %last
    %end = load i64 %at
    %sum = binary add i64 %first, %end
    return i64 %sum
}

function @pick(%which : i64) -> i64 [unwind=no] {
  slot $kept : obj<16x8>
  slot $pair : obj<16x8>
  slot $wide : obj<64x8>
  slot $word : i64

  block ^entry:
    %keep = addr $kept
    store i64 100, %keep
    %keep_end = index i8 [projection=field] %keep, 8
    store i64 %which, %keep_end
    switch %which, ^other, 0:^first, 1:^second

  block ^first:
    %pair = addr $pair
    store i64 3, %pair
    %pair_end = index i8 [projection=field] %pair, 8
    store i64 4, %pair_end
    %from_pair = call i64 @ends(%pair, 1)
    jump ^done

  block ^second:
    %wide = addr $wide
    zeroinit 64x8 %wide
    store i64 10, %wide
    %wide_end = index i8 [projection=field] %wide, 56
    store i64 20, %wide_end
    %from_wide = call i64 @ends(%wide, 7)
    jump ^done

  block ^other:
    %word = addr $word
    store i64 %which, %word
    %from_word = call i64 @ends(%word, 0)
    jump ^done

  block ^done:
    %part = phi i64 [^first: %from_pair, ^second: %from_wide, ^other: %from_word]
    %base = call i64 @ends(%keep, 1)
    %total = binary add i64 %part, %base
    return i64 %total
}

function @main() -> i64 [role=entry] {
  block ^entry:
    %zero = call i64 @pick(0)
    %one = call i64 @pick(1)
    %five = call i64 @pick(5)
    %zero_bad = cmp ne i64 %zero, 107
    %one_bad = cmp ne i64 %one, 131
    %five_bad = cmp ne i64 %five, 115
    %bad0 = binary or i64 %zero_bad, %one_bad
    %bad = binary or i64 %bad0, %five_bad
    return i64 %bad
}

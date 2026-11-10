# A memory value multiplied by an imm64 needs no separate loaded operand.
global @input : i64 = -1

function @memory_product(%p : ptr) -> i64 [unwind=no] {
  block ^entry:
    %value = load i64 %p
    %product = binary mul i64 %value, 6364136223846793005
    return i64 %product
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %product = call i64 @memory_product(@input)
    %bad = cmp ne i64 %product, -6364136223846793005
    %result = convert trunc i32 i64 %bad
    return i32 %result
}

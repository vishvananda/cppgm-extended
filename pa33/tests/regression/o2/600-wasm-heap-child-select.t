# On the Wasm target the smaller child of a heap is a select, though the next
# load reads through the chosen index.
target wasm32 1

function @sift(%keys : ptr, %size : u32) -> u32 [no_inline=yes] {
  block ^entry:
    jump ^loop
  block ^loop:
    %at = phi u32 [^entry: 0, ^next: %child]
    %twice = binary mul u32 %at, 2
    %left = binary add u32 %twice, 1
    %right = binary add u32 %twice, 2
    %more = cmp ult u32 %right, %size
    branch %more, ^both, ^done
  block ^both:
    %left_at = index u32 %keys, %left
    %left_key = load u32 %left_at
    %right_at = index u32 %keys, %right
    %right_key = load u32 %right_at
    %less = cmp ult u32 %right_key, %left_key
    branch %less, ^then, ^next
  block ^then:
    jump ^next
  block ^next:
    %child = phi u32 [^then: %right, ^both: %left]
    jump ^loop
  block ^done:
    return u32 %at
}

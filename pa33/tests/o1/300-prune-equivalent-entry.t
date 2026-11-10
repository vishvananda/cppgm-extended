# Equivalence does not keep an unused weak entry alive or leave a dangling fact.
function @implementation(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %value = binary add i64 %x, 7
    return i64 %value
}
function @unused(%x : i64) -> i64 [binding=weak] {
  block ^entry:
    %value = binary add i64 %x, 7
    return i64 %value
}
function @main() -> i64 {
  block ^entry:
    return i64 0
}
equivalent_entry @unused = @implementation

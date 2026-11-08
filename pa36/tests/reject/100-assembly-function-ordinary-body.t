declare function @invalid() -> i64
assembly_function @invalid {
 block ^entry:
  return i64 7
}

int example(int input) {
  auto closure = [assigned = input, direct(input), listed{input}](auto argument) {
    return assigned + direct + listed + argument;
  };
  return closure(1);
}

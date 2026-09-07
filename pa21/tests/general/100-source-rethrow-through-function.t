int f() {
  try {
    throw 3;
  } catch (int) {
    throw;
  }
}

int main() {
  try {
    return f();
  } catch (int x) {
    return x - 3;
  }
}

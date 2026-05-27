struct NonTrivialCopy {
  NonTrivialCopy() = default;
  NonTrivialCopy(const NonTrivialCopy &);
};

struct TrivialCopy {
  TrivialCopy() = default;
  TrivialCopy(const TrivialCopy &) = default;
};

static_assert(__is_trivially_constructible(int, double), "scalar conversion");
static_assert(__is_trivially_constructible(int &, int &), "reference binding");
static_assert(__is_trivially_constructible(TrivialCopy), "default construction");
static_assert(__is_trivially_constructible(TrivialCopy, const TrivialCopy &), "copy construction");
static_assert(!__is_trivially_constructible(NonTrivialCopy, const NonTrivialCopy &),
              "non-trivial copy construction");

int main() { return 0; }

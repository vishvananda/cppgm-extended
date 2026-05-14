int sink;

void run(bool cond)
{
  int substitution_depth = 0;
  bool structural_substitution_failure = false;

  const auto mark_structural_substitution_failure =
      [&](int value) -> void
  {
    if(substitution_depth != 1 || !cond) {
      return;
    }
    structural_substitution_failure = true;
    sink = value;
  };

  struct ScopedSubstitutionDepth
  {
    explicit ScopedSubstitutionDepth(int & depth_in)
      : depth(depth_in)
    {
      ++depth;
    }

    ~ScopedSubstitutionDepth()
    {
      --depth;
    }

    int & depth;
  };

  const auto substitute =
      [&](int value) -> bool
  {
    ScopedSubstitutionDepth scoped_substitution_depth(substitution_depth);
    mark_structural_substitution_failure(value);
    return structural_substitution_failure;
  };

  substitute(17);
}

int main()
{
  run(true);
  return sink == 17 ? 0 : 1;
}

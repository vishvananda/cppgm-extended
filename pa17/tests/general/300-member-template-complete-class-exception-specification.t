struct holder
{
  template<class T> int current_size() noexcept(sizeof(holder) > 0)
  {
    return 7;
  }
  // This dependent specification is checked only when the member is needed.
  template<class T> void unused() noexcept(sizeof(T) > 0);
};

static_assert(noexcept(holder().current_size<int>()), "complete class specification");
int main() { return holder().current_size<int>() == 7 ? 0 : 1; }

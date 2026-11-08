struct Item { constexpr bool valid() const { return true; } void take(int() noexcept(valid())); };

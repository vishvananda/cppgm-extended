// N3485 5.19: compact aggregate storage does not permit inactive union member reads. expect reject
union Choice { int first; unsigned second; };
struct Outer { Choice choice; int tail; };
constexpr Outer value = {{1}, 2};
constexpr unsigned invalid = value.choice.second;

// N3485 5.19p2: omitted elements activate only the first union member.
// expect reject
union U { int first; long second; };
constexpr const U (&values)[3] = {};
static_assert(values[2].first == 0, "active zero member");
static_assert(values[2].second == 0, "inactive member cannot be read");

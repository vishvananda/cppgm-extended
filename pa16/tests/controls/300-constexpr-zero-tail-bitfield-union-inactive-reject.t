// N3485 5.19p2: a zero bit-field does not activate the other union member.
// expect reject
union U { int first; unsigned second : 3; };
constexpr const U (&values)[3] = {};
static_assert(values[2].first == 0, "active zero member");
static_assert(values[2].second == 0, "inactive bit-field cannot be read");

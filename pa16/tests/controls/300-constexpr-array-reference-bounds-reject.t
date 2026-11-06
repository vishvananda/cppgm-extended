// N3485 5.19p2: omitted values are zero only inside the array. expect reject
constexpr const int (&values)[3] = {};
static_assert(values[3] == 0, "outside array");
int main() { return 0; }

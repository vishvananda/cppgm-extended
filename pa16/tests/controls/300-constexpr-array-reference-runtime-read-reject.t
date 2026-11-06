// N3485 5.19p2: a dynamic initializer does not give a usable constant
// array value merely because its reference extends a temporary. expect reject
int next();
const int (&values)[3] = {next()};
constexpr int value = values[0];
int main() { return value; }

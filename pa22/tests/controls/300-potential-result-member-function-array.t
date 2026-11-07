// N3485 focus: 3.2 [basic.def.odr]; adopted CWG1472/1741/1926/2083/2170.
// expect exit 0
struct State { int count; void first(); void second(); using Function = void (State::*)(); static constexpr Function functions[2] = {&State::first,&State::second}; void sync(); }; void State::first() { count += 2; } void State::second() { count += 5; } void State::sync() { for (int i = 0; i < 2; ++i) (this->*functions[i])(); } int main() { State state = {0}; state.sync(); state.sync(); return state.count != 14; }

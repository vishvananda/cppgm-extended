// A braced linkage-specification does not make its declarations extern.
extern "C" {
int& missing_initializer;
}

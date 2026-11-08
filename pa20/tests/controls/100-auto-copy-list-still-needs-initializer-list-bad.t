// expect reject
// I13: N3485 8.5.4p2/N3922: copy-list auto still requires the initializer_list declaration.
int main() { auto item = {7}; return 0; }

// 8.5.4p3 and 8.5.3: direct binding cannot discard the literal array's
// const qualification or convert its lvalue category. expect reject
char (&&text)[4] = {"abc"};
int main() { return 0; }

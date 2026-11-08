// expect reject
int data[2];
int* begin(int) { return data; }
int* end(int) { return data + 2; }
int main() { for (int value : 1) {} }

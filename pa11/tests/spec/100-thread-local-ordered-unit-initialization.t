// Accessing the second definition initializes the ordered TLS unit in source order.
int sequence;
int first() { sequence = sequence * 10 + 1; return 7; }
int second() { sequence = sequence * 10 + 2; return 9; }
thread_local int a = first();
thread_local int b = second();
int main() { return b != 9 || a != 7 || sequence != 12; }

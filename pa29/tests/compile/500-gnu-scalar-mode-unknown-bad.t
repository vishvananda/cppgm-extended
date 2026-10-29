// An unrecognized machine-mode name is rejected, never ignored.
typedef unsigned Wrong __attribute__((mode(not_a_machine_mode)));
int main(){return sizeof(Wrong);}

// N3485 8.3.6p3, CWG974: actual lambda defaults are valid; defaults nested in parameter types are not.
// expect exit 0
int main(){return [](int n=0){return n;}();}

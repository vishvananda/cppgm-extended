// N3485 8.3.6p3, CWG974: actual lambda defaults are valid; defaults nested in parameter types are not.
// expect reject
int main(){return [](int(*cb)(int = 1)=0){return cb?cb(0):0;}();}

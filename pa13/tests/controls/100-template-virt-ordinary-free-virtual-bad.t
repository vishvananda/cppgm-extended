// N3485 focus: 7.1.2p5: virtual-function declaration ownership.
// expect reject
virtual int f(){return 0;} int main(){return f();}

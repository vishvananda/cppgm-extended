typedef unsigned long long U;
extern "C" { U reads=0,produces=0,throws=0; }
extern "C" U host_read(const U* p) { ++reads; return *p; }
extern "C" void host_throw(const U* p,U v) { ++throws; if(v&1) throw unsigned(*p); }
extern "C" U host_produce(U v) { ++produces; return v^0x12345678ULL; }

extern "C" int host_alignment(const void* pointer,int bytes) { return bytes>=16 && (reinterpret_cast<unsigned long>(pointer)&15)!=0; }

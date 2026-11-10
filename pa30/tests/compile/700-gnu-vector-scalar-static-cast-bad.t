typedef char Bytes __attribute__((vector_size(16)));
Bytes make(char value){return static_cast<Bytes>(value);}

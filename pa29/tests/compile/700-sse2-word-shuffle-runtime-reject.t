using words=short __attribute__((vector_size(16)));
words bad(words value,int selector){return __builtin_ia32_pshuflw(value,selector);}

extern "C" void vex_source_words(const unsigned* left,const unsigned* right,unsigned* out) {
    asm volatile("vmovdqu (%0),%%ymm0\nvmovdqu (%1),%%ymm9\n"
                 "vpaddd %%ymm9,%%ymm0,%%ymm15\nvmovdqu %%ymm15,(%2)\nvzeroupper"
        : : "r"(left),"r"(right),"r"(out) : "ymm0","ymm9","ymm15","memory");
}

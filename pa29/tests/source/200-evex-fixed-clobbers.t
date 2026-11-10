extern "C" __attribute__((target("avx512f"))) void evex_source_words(const unsigned* left,const unsigned* right,unsigned* out,unsigned mask) {
    asm volatile("vmovdqu32 (%0),%%zmm29\nvmovdqu32 (%1),%%zmm30\nkmovw %3,%%k3\n"
                 "vpaddd %%zmm30,%%zmm29,%%zmm31%{%%k3%}%{z%}\n"
                 "vmovdqu32 %%zmm31,(%2)\nvzeroupper"
        : : "r"(left),"r"(right),"r"(out),"r"(mask)
        : "zmm29","zmm30","zmm31","k3","memory");
}

declare function @evex_zero(%left : ptr, %right : ptr, %out : ptr, %mask : u32) -> void
assembly @evex_zero_body intel readwrite 1 0 0 0 { text "vmovdqu32 zmm29,[rdi]\nvmovdqu32 zmm30,[rsi]\nkmovw k3,ecx\nvpaddd zmm31{k3}{z},zmm29,zmm30\nvmovdqu32 [rdx],zmm31\nvzeroupper\nret" }
assembly_function @evex_zero {
 block ^entry:
  asm @evex_zero_body()
  unreachable
}
declare function @evex_merge(%left : ptr, %right : ptr, %out : ptr, %mask : u32) -> void
assembly @evex_merge_body intel readwrite 1 0 0 0 { text "vmovdqu32 zmm29,[rdi]\nvmovdqu32 zmm30,[rsi]\nkmovw k3,ecx\nvmovdqu32 zmm31,[rdx]\nvpaddd zmm31{k3},zmm29,zmm30\nvmovdqu32 [rdx],zmm31\nvzeroupper\nret" }
assembly_function @evex_merge {
 block ^entry:
  asm @evex_merge_body()
  unreachable
}
declare function @evex_broadcast(%left : ptr, %bias : ptr, %out : ptr) -> void
assembly @evex_broadcast_body intel readwrite 1 0 0 0 { text "vmovdqu32 zmm29,[rdi]\nvpaddd zmm31,zmm29,dword ptr [rsi+508]{1to16}\nvpaddd zmm31,zmm31,dword ptr [rsi+509]{1to16}\nvmovdqu32 [rdx],zmm31\nvzeroupper\nret" }
assembly_function @evex_broadcast {
 block ^entry:
  asm @evex_broadcast_body()
  unreachable
}

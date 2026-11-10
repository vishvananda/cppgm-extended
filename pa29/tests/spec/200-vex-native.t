declare function @vex_words(%left : ptr, %right : ptr, %out : ptr) -> void
assembly @vex_words_body intel none 1 0 0 0 { text "vmovdqu ymm0,[rdi]\nvmovdqu ymm9,[rsi]\nvpaddd ymm15,ymm0,ymm9\nvpslld ymm15,ymm15,3\nvpsrld ymm15,ymm15,1\nvpxor ymm15,ymm15,ymm9\nvmovdqu [rdx],ymm15\nvzeroupper\nret" }
assembly_function @vex_words {
 block ^entry:
  asm @vex_words_body()
  unreachable
}
declare function @vex_lanes(%input : ptr, %out : ptr) -> void
assembly @vex_lanes_body intel none 1 0 0 0 { text "vmovdqu ymm8,[rdi]\nvpshufd ymm9,ymm8,27\nvextracti128 xmm15,ymm9,1\nvmovdqu [rsi],xmm15\nvperm2i128 ymm10,ymm8,ymm8,1\nvinserti128 ymm10,ymm10,xmm15,0\nvmovdqu [rsi+16],ymm10\nvzeroupper\nret" }
assembly_function @vex_lanes {
 block ^entry:
  asm @vex_lanes_body()
  unreachable
}
declare function @vex_bytes(%input : ptr, %bias : ptr, %out : ptr) -> void
assembly @vex_bytes_body intel none 1 0 0 0 { text "vmovdqu ymm0,[rdi]\nvpabsb ymm1,ymm0\nvpbroadcastb ymm9,byte ptr [rsi]\nvpaddb ymm15,ymm1,ymm9\nvmovdqu [rdx],ymm15\nvzeroupper\nret" }
assembly_function @vex_bytes {
 block ^entry:
  asm @vex_bytes_body()
  unreachable
}
declare function @vex_scalar(%input : ptr, %out : ptr) -> void
assembly @vex_scalar_body intel none 1 0 0 0 { text "vmovupd xmm9,[rdi]\nvmovupd xmm14,[rdi+16]\nvmovsd xmm15,xmm9,xmm14\nvmovupd [rsi],xmm15\nvaddsd xmm15,xmm9,qword ptr [rdi+32]\nvmovupd [rsi+16],xmm15\nvzeroupper\nret" }
assembly_function @vex_scalar {
 block ^entry:
  asm @vex_scalar_body()
  unreachable
}
declare function @vex_fused(%left : ptr, %right : ptr, %out : ptr) -> void
assembly @vex_fused_body intel none 1 0 0 0 { text "vmovups ymm8,[rdi]\nvmovups ymm9,[rsi]\nvmovups ymm15,[rdx]\nvfmadd231ps ymm15,ymm8,ymm9\nvmovups [rdx],ymm15\nvzeroupper\nret" }
assembly_function @vex_fused {
 block ^entry:
  asm @vex_fused_body()
  unreachable
}

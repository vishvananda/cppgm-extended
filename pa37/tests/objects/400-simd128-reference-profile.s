.text

.section .text.simd_v128_load,"",@
.globl simd_v128_load
simd_v128_load:
 .functype simd_v128_load (i32) -> (v128)
 local.get 0
 v128.load 0
 end_function

.section .text.simd_v128_load8x8_s,"",@
.globl simd_v128_load8x8_s
simd_v128_load8x8_s:
 .functype simd_v128_load8x8_s (i32) -> (v128)
 local.get 0
 i16x8.load8x8_s 0
 end_function

.section .text.simd_v128_load8x8_u,"",@
.globl simd_v128_load8x8_u
simd_v128_load8x8_u:
 .functype simd_v128_load8x8_u (i32) -> (v128)
 local.get 0
 i16x8.load8x8_u 0
 end_function

.section .text.simd_v128_load16x4_s,"",@
.globl simd_v128_load16x4_s
simd_v128_load16x4_s:
 .functype simd_v128_load16x4_s (i32) -> (v128)
 local.get 0
 i32x4.load16x4_s 0
 end_function

.section .text.simd_v128_load16x4_u,"",@
.globl simd_v128_load16x4_u
simd_v128_load16x4_u:
 .functype simd_v128_load16x4_u (i32) -> (v128)
 local.get 0
 i32x4.load16x4_u 0
 end_function

.section .text.simd_v128_load32x2_s,"",@
.globl simd_v128_load32x2_s
simd_v128_load32x2_s:
 .functype simd_v128_load32x2_s (i32) -> (v128)
 local.get 0
 i64x2.load32x2_s 0
 end_function

.section .text.simd_v128_load32x2_u,"",@
.globl simd_v128_load32x2_u
simd_v128_load32x2_u:
 .functype simd_v128_load32x2_u (i32) -> (v128)
 local.get 0
 i64x2.load32x2_u 0
 end_function

.section .text.simd_v128_load8_splat,"",@
.globl simd_v128_load8_splat
simd_v128_load8_splat:
 .functype simd_v128_load8_splat (i32) -> (v128)
 local.get 0
 v128.load8_splat 0
 end_function

.section .text.simd_v128_load16_splat,"",@
.globl simd_v128_load16_splat
simd_v128_load16_splat:
 .functype simd_v128_load16_splat (i32) -> (v128)
 local.get 0
 v128.load16_splat 0
 end_function

.section .text.simd_v128_load32_splat,"",@
.globl simd_v128_load32_splat
simd_v128_load32_splat:
 .functype simd_v128_load32_splat (i32) -> (v128)
 local.get 0
 v128.load32_splat 0
 end_function

.section .text.simd_v128_load64_splat,"",@
.globl simd_v128_load64_splat
simd_v128_load64_splat:
 .functype simd_v128_load64_splat (i32) -> (v128)
 local.get 0
 v128.load64_splat 0
 end_function

.section .text.simd_v128_store,"",@
.globl simd_v128_store
simd_v128_store:
 .functype simd_v128_store (i32, v128) -> ()
 local.get 0
 local.get 1
 v128.store 0
 end_function

.section .text.simd_v128_const,"",@
.globl simd_v128_const
simd_v128_const:
 .functype simd_v128_const () -> (v128)
 v128.const 0, 0, 0, 0
 end_function

.section .text.simd_i8x16_shuffle,"",@
.globl simd_i8x16_shuffle
simd_i8x16_shuffle:
 .functype simd_i8x16_shuffle (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i8x16.shuffle 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
 end_function

.section .text.simd_i8x16_swizzle,"",@
.globl simd_i8x16_swizzle
simd_i8x16_swizzle:
 .functype simd_i8x16_swizzle (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i8x16.swizzle
 end_function

.section .text.simd_i8x16_splat,"",@
.globl simd_i8x16_splat
simd_i8x16_splat:
 .functype simd_i8x16_splat (i32) -> (v128)
 local.get 0
 i8x16.splat
 end_function

.section .text.simd_i16x8_splat,"",@
.globl simd_i16x8_splat
simd_i16x8_splat:
 .functype simd_i16x8_splat (i32) -> (v128)
 local.get 0
 i16x8.splat
 end_function

.section .text.simd_i32x4_splat,"",@
.globl simd_i32x4_splat
simd_i32x4_splat:
 .functype simd_i32x4_splat (i32) -> (v128)
 local.get 0
 i32x4.splat
 end_function

.section .text.simd_i64x2_splat,"",@
.globl simd_i64x2_splat
simd_i64x2_splat:
 .functype simd_i64x2_splat (i64) -> (v128)
 local.get 0
 i64x2.splat
 end_function

.section .text.simd_f32x4_splat,"",@
.globl simd_f32x4_splat
simd_f32x4_splat:
 .functype simd_f32x4_splat (f32) -> (v128)
 local.get 0
 f32x4.splat
 end_function

.section .text.simd_f64x2_splat,"",@
.globl simd_f64x2_splat
simd_f64x2_splat:
 .functype simd_f64x2_splat (f64) -> (v128)
 local.get 0
 f64x2.splat
 end_function

.section .text.simd_i8x16_extract_lane_s,"",@
.globl simd_i8x16_extract_lane_s
simd_i8x16_extract_lane_s:
 .functype simd_i8x16_extract_lane_s (v128) -> (i32)
 local.get 0
 i8x16.extract_lane_s 0
 end_function

.section .text.simd_i8x16_extract_lane_u,"",@
.globl simd_i8x16_extract_lane_u
simd_i8x16_extract_lane_u:
 .functype simd_i8x16_extract_lane_u (v128) -> (i32)
 local.get 0
 i8x16.extract_lane_u 0
 end_function

.section .text.simd_i8x16_replace_lane,"",@
.globl simd_i8x16_replace_lane
simd_i8x16_replace_lane:
 .functype simd_i8x16_replace_lane (v128, i32) -> (v128)
 local.get 0
 local.get 1
 i8x16.replace_lane 0
 end_function

.section .text.simd_i16x8_extract_lane_s,"",@
.globl simd_i16x8_extract_lane_s
simd_i16x8_extract_lane_s:
 .functype simd_i16x8_extract_lane_s (v128) -> (i32)
 local.get 0
 i16x8.extract_lane_s 0
 end_function

.section .text.simd_i16x8_extract_lane_u,"",@
.globl simd_i16x8_extract_lane_u
simd_i16x8_extract_lane_u:
 .functype simd_i16x8_extract_lane_u (v128) -> (i32)
 local.get 0
 i16x8.extract_lane_u 0
 end_function

.section .text.simd_i16x8_replace_lane,"",@
.globl simd_i16x8_replace_lane
simd_i16x8_replace_lane:
 .functype simd_i16x8_replace_lane (v128, i32) -> (v128)
 local.get 0
 local.get 1
 i16x8.replace_lane 0
 end_function

.section .text.simd_i32x4_extract_lane,"",@
.globl simd_i32x4_extract_lane
simd_i32x4_extract_lane:
 .functype simd_i32x4_extract_lane (v128) -> (i32)
 local.get 0
 i32x4.extract_lane 0
 end_function

.section .text.simd_i32x4_replace_lane,"",@
.globl simd_i32x4_replace_lane
simd_i32x4_replace_lane:
 .functype simd_i32x4_replace_lane (v128, i32) -> (v128)
 local.get 0
 local.get 1
 i32x4.replace_lane 0
 end_function

.section .text.simd_i64x2_extract_lane,"",@
.globl simd_i64x2_extract_lane
simd_i64x2_extract_lane:
 .functype simd_i64x2_extract_lane (v128) -> (i64)
 local.get 0
 i64x2.extract_lane 0
 end_function

.section .text.simd_i64x2_replace_lane,"",@
.globl simd_i64x2_replace_lane
simd_i64x2_replace_lane:
 .functype simd_i64x2_replace_lane (v128, i64) -> (v128)
 local.get 0
 local.get 1
 i64x2.replace_lane 0
 end_function

.section .text.simd_f32x4_extract_lane,"",@
.globl simd_f32x4_extract_lane
simd_f32x4_extract_lane:
 .functype simd_f32x4_extract_lane (v128) -> (f32)
 local.get 0
 f32x4.extract_lane 0
 end_function

.section .text.simd_f32x4_replace_lane,"",@
.globl simd_f32x4_replace_lane
simd_f32x4_replace_lane:
 .functype simd_f32x4_replace_lane (v128, f32) -> (v128)
 local.get 0
 local.get 1
 f32x4.replace_lane 0
 end_function

.section .text.simd_f64x2_extract_lane,"",@
.globl simd_f64x2_extract_lane
simd_f64x2_extract_lane:
 .functype simd_f64x2_extract_lane (v128) -> (f64)
 local.get 0
 f64x2.extract_lane 0
 end_function

.section .text.simd_f64x2_replace_lane,"",@
.globl simd_f64x2_replace_lane
simd_f64x2_replace_lane:
 .functype simd_f64x2_replace_lane (v128, f64) -> (v128)
 local.get 0
 local.get 1
 f64x2.replace_lane 0
 end_function

.section .text.simd_i8x16_eq,"",@
.globl simd_i8x16_eq
simd_i8x16_eq:
 .functype simd_i8x16_eq (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i8x16.eq
 end_function

.section .text.simd_i8x16_ne,"",@
.globl simd_i8x16_ne
simd_i8x16_ne:
 .functype simd_i8x16_ne (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i8x16.ne
 end_function

.section .text.simd_i8x16_lt_s,"",@
.globl simd_i8x16_lt_s
simd_i8x16_lt_s:
 .functype simd_i8x16_lt_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i8x16.lt_s
 end_function

.section .text.simd_i8x16_lt_u,"",@
.globl simd_i8x16_lt_u
simd_i8x16_lt_u:
 .functype simd_i8x16_lt_u (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i8x16.lt_u
 end_function

.section .text.simd_i8x16_gt_s,"",@
.globl simd_i8x16_gt_s
simd_i8x16_gt_s:
 .functype simd_i8x16_gt_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i8x16.gt_s
 end_function

.section .text.simd_i8x16_gt_u,"",@
.globl simd_i8x16_gt_u
simd_i8x16_gt_u:
 .functype simd_i8x16_gt_u (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i8x16.gt_u
 end_function

.section .text.simd_i8x16_le_s,"",@
.globl simd_i8x16_le_s
simd_i8x16_le_s:
 .functype simd_i8x16_le_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i8x16.le_s
 end_function

.section .text.simd_i8x16_le_u,"",@
.globl simd_i8x16_le_u
simd_i8x16_le_u:
 .functype simd_i8x16_le_u (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i8x16.le_u
 end_function

.section .text.simd_i8x16_ge_s,"",@
.globl simd_i8x16_ge_s
simd_i8x16_ge_s:
 .functype simd_i8x16_ge_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i8x16.ge_s
 end_function

.section .text.simd_i8x16_ge_u,"",@
.globl simd_i8x16_ge_u
simd_i8x16_ge_u:
 .functype simd_i8x16_ge_u (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i8x16.ge_u
 end_function

.section .text.simd_i16x8_eq,"",@
.globl simd_i16x8_eq
simd_i16x8_eq:
 .functype simd_i16x8_eq (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i16x8.eq
 end_function

.section .text.simd_i16x8_ne,"",@
.globl simd_i16x8_ne
simd_i16x8_ne:
 .functype simd_i16x8_ne (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i16x8.ne
 end_function

.section .text.simd_i16x8_lt_s,"",@
.globl simd_i16x8_lt_s
simd_i16x8_lt_s:
 .functype simd_i16x8_lt_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i16x8.lt_s
 end_function

.section .text.simd_i16x8_lt_u,"",@
.globl simd_i16x8_lt_u
simd_i16x8_lt_u:
 .functype simd_i16x8_lt_u (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i16x8.lt_u
 end_function

.section .text.simd_i16x8_gt_s,"",@
.globl simd_i16x8_gt_s
simd_i16x8_gt_s:
 .functype simd_i16x8_gt_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i16x8.gt_s
 end_function

.section .text.simd_i16x8_gt_u,"",@
.globl simd_i16x8_gt_u
simd_i16x8_gt_u:
 .functype simd_i16x8_gt_u (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i16x8.gt_u
 end_function

.section .text.simd_i16x8_le_s,"",@
.globl simd_i16x8_le_s
simd_i16x8_le_s:
 .functype simd_i16x8_le_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i16x8.le_s
 end_function

.section .text.simd_i16x8_le_u,"",@
.globl simd_i16x8_le_u
simd_i16x8_le_u:
 .functype simd_i16x8_le_u (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i16x8.le_u
 end_function

.section .text.simd_i16x8_ge_s,"",@
.globl simd_i16x8_ge_s
simd_i16x8_ge_s:
 .functype simd_i16x8_ge_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i16x8.ge_s
 end_function

.section .text.simd_i16x8_ge_u,"",@
.globl simd_i16x8_ge_u
simd_i16x8_ge_u:
 .functype simd_i16x8_ge_u (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i16x8.ge_u
 end_function

.section .text.simd_i32x4_eq,"",@
.globl simd_i32x4_eq
simd_i32x4_eq:
 .functype simd_i32x4_eq (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i32x4.eq
 end_function

.section .text.simd_i32x4_ne,"",@
.globl simd_i32x4_ne
simd_i32x4_ne:
 .functype simd_i32x4_ne (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i32x4.ne
 end_function

.section .text.simd_i32x4_lt_s,"",@
.globl simd_i32x4_lt_s
simd_i32x4_lt_s:
 .functype simd_i32x4_lt_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i32x4.lt_s
 end_function

.section .text.simd_i32x4_lt_u,"",@
.globl simd_i32x4_lt_u
simd_i32x4_lt_u:
 .functype simd_i32x4_lt_u (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i32x4.lt_u
 end_function

.section .text.simd_i32x4_gt_s,"",@
.globl simd_i32x4_gt_s
simd_i32x4_gt_s:
 .functype simd_i32x4_gt_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i32x4.gt_s
 end_function

.section .text.simd_i32x4_gt_u,"",@
.globl simd_i32x4_gt_u
simd_i32x4_gt_u:
 .functype simd_i32x4_gt_u (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i32x4.gt_u
 end_function

.section .text.simd_i32x4_le_s,"",@
.globl simd_i32x4_le_s
simd_i32x4_le_s:
 .functype simd_i32x4_le_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i32x4.le_s
 end_function

.section .text.simd_i32x4_le_u,"",@
.globl simd_i32x4_le_u
simd_i32x4_le_u:
 .functype simd_i32x4_le_u (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i32x4.le_u
 end_function

.section .text.simd_i32x4_ge_s,"",@
.globl simd_i32x4_ge_s
simd_i32x4_ge_s:
 .functype simd_i32x4_ge_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i32x4.ge_s
 end_function

.section .text.simd_i32x4_ge_u,"",@
.globl simd_i32x4_ge_u
simd_i32x4_ge_u:
 .functype simd_i32x4_ge_u (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i32x4.ge_u
 end_function

.section .text.simd_f32x4_eq,"",@
.globl simd_f32x4_eq
simd_f32x4_eq:
 .functype simd_f32x4_eq (v128, v128) -> (v128)
 local.get 0
 local.get 1
 f32x4.eq
 end_function

.section .text.simd_f32x4_ne,"",@
.globl simd_f32x4_ne
simd_f32x4_ne:
 .functype simd_f32x4_ne (v128, v128) -> (v128)
 local.get 0
 local.get 1
 f32x4.ne
 end_function

.section .text.simd_f32x4_lt,"",@
.globl simd_f32x4_lt
simd_f32x4_lt:
 .functype simd_f32x4_lt (v128, v128) -> (v128)
 local.get 0
 local.get 1
 f32x4.lt
 end_function

.section .text.simd_f32x4_gt,"",@
.globl simd_f32x4_gt
simd_f32x4_gt:
 .functype simd_f32x4_gt (v128, v128) -> (v128)
 local.get 0
 local.get 1
 f32x4.gt
 end_function

.section .text.simd_f32x4_le,"",@
.globl simd_f32x4_le
simd_f32x4_le:
 .functype simd_f32x4_le (v128, v128) -> (v128)
 local.get 0
 local.get 1
 f32x4.le
 end_function

.section .text.simd_f32x4_ge,"",@
.globl simd_f32x4_ge
simd_f32x4_ge:
 .functype simd_f32x4_ge (v128, v128) -> (v128)
 local.get 0
 local.get 1
 f32x4.ge
 end_function

.section .text.simd_f64x2_eq,"",@
.globl simd_f64x2_eq
simd_f64x2_eq:
 .functype simd_f64x2_eq (v128, v128) -> (v128)
 local.get 0
 local.get 1
 f64x2.eq
 end_function

.section .text.simd_f64x2_ne,"",@
.globl simd_f64x2_ne
simd_f64x2_ne:
 .functype simd_f64x2_ne (v128, v128) -> (v128)
 local.get 0
 local.get 1
 f64x2.ne
 end_function

.section .text.simd_f64x2_lt,"",@
.globl simd_f64x2_lt
simd_f64x2_lt:
 .functype simd_f64x2_lt (v128, v128) -> (v128)
 local.get 0
 local.get 1
 f64x2.lt
 end_function

.section .text.simd_f64x2_gt,"",@
.globl simd_f64x2_gt
simd_f64x2_gt:
 .functype simd_f64x2_gt (v128, v128) -> (v128)
 local.get 0
 local.get 1
 f64x2.gt
 end_function

.section .text.simd_f64x2_le,"",@
.globl simd_f64x2_le
simd_f64x2_le:
 .functype simd_f64x2_le (v128, v128) -> (v128)
 local.get 0
 local.get 1
 f64x2.le
 end_function

.section .text.simd_f64x2_ge,"",@
.globl simd_f64x2_ge
simd_f64x2_ge:
 .functype simd_f64x2_ge (v128, v128) -> (v128)
 local.get 0
 local.get 1
 f64x2.ge
 end_function

.section .text.simd_v128_not,"",@
.globl simd_v128_not
simd_v128_not:
 .functype simd_v128_not (v128) -> (v128)
 local.get 0
 v128.not
 end_function

.section .text.simd_v128_and,"",@
.globl simd_v128_and
simd_v128_and:
 .functype simd_v128_and (v128, v128) -> (v128)
 local.get 0
 local.get 1
 v128.and
 end_function

.section .text.simd_v128_andnot,"",@
.globl simd_v128_andnot
simd_v128_andnot:
 .functype simd_v128_andnot (v128, v128) -> (v128)
 local.get 0
 local.get 1
 v128.andnot
 end_function

.section .text.simd_v128_or,"",@
.globl simd_v128_or
simd_v128_or:
 .functype simd_v128_or (v128, v128) -> (v128)
 local.get 0
 local.get 1
 v128.or
 end_function

.section .text.simd_v128_xor,"",@
.globl simd_v128_xor
simd_v128_xor:
 .functype simd_v128_xor (v128, v128) -> (v128)
 local.get 0
 local.get 1
 v128.xor
 end_function

.section .text.simd_v128_bitselect,"",@
.globl simd_v128_bitselect
simd_v128_bitselect:
 .functype simd_v128_bitselect (v128, v128, v128) -> (v128)
 local.get 0
 local.get 1
 local.get 2
 v128.bitselect
 end_function

.section .text.simd_v128_any_true,"",@
.globl simd_v128_any_true
simd_v128_any_true:
 .functype simd_v128_any_true (v128) -> (i32)
 local.get 0
 v128.any_true
 end_function

.section .text.simd_v128_load8_lane,"",@
.globl simd_v128_load8_lane
simd_v128_load8_lane:
 .functype simd_v128_load8_lane (i32, v128) -> (v128)
 local.get 0
 local.get 1
 v128.load8_lane 0, 0
 end_function

.section .text.simd_v128_load16_lane,"",@
.globl simd_v128_load16_lane
simd_v128_load16_lane:
 .functype simd_v128_load16_lane (i32, v128) -> (v128)
 local.get 0
 local.get 1
 v128.load16_lane 0, 0
 end_function

.section .text.simd_v128_load32_lane,"",@
.globl simd_v128_load32_lane
simd_v128_load32_lane:
 .functype simd_v128_load32_lane (i32, v128) -> (v128)
 local.get 0
 local.get 1
 v128.load32_lane 0, 0
 end_function

.section .text.simd_v128_load64_lane,"",@
.globl simd_v128_load64_lane
simd_v128_load64_lane:
 .functype simd_v128_load64_lane (i32, v128) -> (v128)
 local.get 0
 local.get 1
 v128.load64_lane 0, 0
 end_function

.section .text.simd_v128_store8_lane,"",@
.globl simd_v128_store8_lane
simd_v128_store8_lane:
 .functype simd_v128_store8_lane (i32, v128) -> ()
 local.get 0
 local.get 1
 v128.store8_lane 0, 0
 end_function

.section .text.simd_v128_store16_lane,"",@
.globl simd_v128_store16_lane
simd_v128_store16_lane:
 .functype simd_v128_store16_lane (i32, v128) -> ()
 local.get 0
 local.get 1
 v128.store16_lane 0, 0
 end_function

.section .text.simd_v128_store32_lane,"",@
.globl simd_v128_store32_lane
simd_v128_store32_lane:
 .functype simd_v128_store32_lane (i32, v128) -> ()
 local.get 0
 local.get 1
 v128.store32_lane 0, 0
 end_function

.section .text.simd_v128_store64_lane,"",@
.globl simd_v128_store64_lane
simd_v128_store64_lane:
 .functype simd_v128_store64_lane (i32, v128) -> ()
 local.get 0
 local.get 1
 v128.store64_lane 0, 0
 end_function

.section .text.simd_v128_load32_zero,"",@
.globl simd_v128_load32_zero
simd_v128_load32_zero:
 .functype simd_v128_load32_zero (i32) -> (v128)
 local.get 0
 v128.load32_zero 0
 end_function

.section .text.simd_v128_load64_zero,"",@
.globl simd_v128_load64_zero
simd_v128_load64_zero:
 .functype simd_v128_load64_zero (i32) -> (v128)
 local.get 0
 v128.load64_zero 0
 end_function

.section .text.simd_f32x4_demote_f64x2_zero,"",@
.globl simd_f32x4_demote_f64x2_zero
simd_f32x4_demote_f64x2_zero:
 .functype simd_f32x4_demote_f64x2_zero (v128) -> (v128)
 local.get 0
 f32x4.demote_f64x2_zero
 end_function

.section .text.simd_f64x2_promote_low_f32x4,"",@
.globl simd_f64x2_promote_low_f32x4
simd_f64x2_promote_low_f32x4:
 .functype simd_f64x2_promote_low_f32x4 (v128) -> (v128)
 local.get 0
 f64x2.promote_low_f32x4
 end_function

.section .text.simd_i8x16_abs,"",@
.globl simd_i8x16_abs
simd_i8x16_abs:
 .functype simd_i8x16_abs (v128) -> (v128)
 local.get 0
 i8x16.abs
 end_function

.section .text.simd_i8x16_neg,"",@
.globl simd_i8x16_neg
simd_i8x16_neg:
 .functype simd_i8x16_neg (v128) -> (v128)
 local.get 0
 i8x16.neg
 end_function

.section .text.simd_i8x16_popcnt,"",@
.globl simd_i8x16_popcnt
simd_i8x16_popcnt:
 .functype simd_i8x16_popcnt (v128) -> (v128)
 local.get 0
 i8x16.popcnt
 end_function

.section .text.simd_i8x16_all_true,"",@
.globl simd_i8x16_all_true
simd_i8x16_all_true:
 .functype simd_i8x16_all_true (v128) -> (i32)
 local.get 0
 i8x16.all_true
 end_function

.section .text.simd_i8x16_bitmask,"",@
.globl simd_i8x16_bitmask
simd_i8x16_bitmask:
 .functype simd_i8x16_bitmask (v128) -> (i32)
 local.get 0
 i8x16.bitmask
 end_function

.section .text.simd_i8x16_narrow_i16x8_s,"",@
.globl simd_i8x16_narrow_i16x8_s
simd_i8x16_narrow_i16x8_s:
 .functype simd_i8x16_narrow_i16x8_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i8x16.narrow_i16x8_s
 end_function

.section .text.simd_i8x16_narrow_i16x8_u,"",@
.globl simd_i8x16_narrow_i16x8_u
simd_i8x16_narrow_i16x8_u:
 .functype simd_i8x16_narrow_i16x8_u (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i8x16.narrow_i16x8_u
 end_function

.section .text.simd_f32x4_ceil,"",@
.globl simd_f32x4_ceil
simd_f32x4_ceil:
 .functype simd_f32x4_ceil (v128) -> (v128)
 local.get 0
 f32x4.ceil
 end_function

.section .text.simd_f32x4_floor,"",@
.globl simd_f32x4_floor
simd_f32x4_floor:
 .functype simd_f32x4_floor (v128) -> (v128)
 local.get 0
 f32x4.floor
 end_function

.section .text.simd_f32x4_trunc,"",@
.globl simd_f32x4_trunc
simd_f32x4_trunc:
 .functype simd_f32x4_trunc (v128) -> (v128)
 local.get 0
 f32x4.trunc
 end_function

.section .text.simd_f32x4_nearest,"",@
.globl simd_f32x4_nearest
simd_f32x4_nearest:
 .functype simd_f32x4_nearest (v128) -> (v128)
 local.get 0
 f32x4.nearest
 end_function

.section .text.simd_i8x16_shl,"",@
.globl simd_i8x16_shl
simd_i8x16_shl:
 .functype simd_i8x16_shl (v128, i32) -> (v128)
 local.get 0
 local.get 1
 i8x16.shl
 end_function

.section .text.simd_i8x16_shr_s,"",@
.globl simd_i8x16_shr_s
simd_i8x16_shr_s:
 .functype simd_i8x16_shr_s (v128, i32) -> (v128)
 local.get 0
 local.get 1
 i8x16.shr_s
 end_function

.section .text.simd_i8x16_shr_u,"",@
.globl simd_i8x16_shr_u
simd_i8x16_shr_u:
 .functype simd_i8x16_shr_u (v128, i32) -> (v128)
 local.get 0
 local.get 1
 i8x16.shr_u
 end_function

.section .text.simd_i8x16_add,"",@
.globl simd_i8x16_add
simd_i8x16_add:
 .functype simd_i8x16_add (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i8x16.add
 end_function

.section .text.simd_i8x16_add_sat_s,"",@
.globl simd_i8x16_add_sat_s
simd_i8x16_add_sat_s:
 .functype simd_i8x16_add_sat_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i8x16.add_sat_s
 end_function

.section .text.simd_i8x16_add_sat_u,"",@
.globl simd_i8x16_add_sat_u
simd_i8x16_add_sat_u:
 .functype simd_i8x16_add_sat_u (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i8x16.add_sat_u
 end_function

.section .text.simd_i8x16_sub,"",@
.globl simd_i8x16_sub
simd_i8x16_sub:
 .functype simd_i8x16_sub (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i8x16.sub
 end_function

.section .text.simd_i8x16_sub_sat_s,"",@
.globl simd_i8x16_sub_sat_s
simd_i8x16_sub_sat_s:
 .functype simd_i8x16_sub_sat_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i8x16.sub_sat_s
 end_function

.section .text.simd_i8x16_sub_sat_u,"",@
.globl simd_i8x16_sub_sat_u
simd_i8x16_sub_sat_u:
 .functype simd_i8x16_sub_sat_u (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i8x16.sub_sat_u
 end_function

.section .text.simd_f64x2_ceil,"",@
.globl simd_f64x2_ceil
simd_f64x2_ceil:
 .functype simd_f64x2_ceil (v128) -> (v128)
 local.get 0
 f64x2.ceil
 end_function

.section .text.simd_f64x2_floor,"",@
.globl simd_f64x2_floor
simd_f64x2_floor:
 .functype simd_f64x2_floor (v128) -> (v128)
 local.get 0
 f64x2.floor
 end_function

.section .text.simd_i8x16_min_s,"",@
.globl simd_i8x16_min_s
simd_i8x16_min_s:
 .functype simd_i8x16_min_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i8x16.min_s
 end_function

.section .text.simd_i8x16_min_u,"",@
.globl simd_i8x16_min_u
simd_i8x16_min_u:
 .functype simd_i8x16_min_u (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i8x16.min_u
 end_function

.section .text.simd_i8x16_max_s,"",@
.globl simd_i8x16_max_s
simd_i8x16_max_s:
 .functype simd_i8x16_max_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i8x16.max_s
 end_function

.section .text.simd_i8x16_max_u,"",@
.globl simd_i8x16_max_u
simd_i8x16_max_u:
 .functype simd_i8x16_max_u (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i8x16.max_u
 end_function

.section .text.simd_f64x2_trunc,"",@
.globl simd_f64x2_trunc
simd_f64x2_trunc:
 .functype simd_f64x2_trunc (v128) -> (v128)
 local.get 0
 f64x2.trunc
 end_function

.section .text.simd_i8x16_avgr_u,"",@
.globl simd_i8x16_avgr_u
simd_i8x16_avgr_u:
 .functype simd_i8x16_avgr_u (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i8x16.avgr_u
 end_function

.section .text.simd_i16x8_extadd_pairwise_i8x16_s,"",@
.globl simd_i16x8_extadd_pairwise_i8x16_s
simd_i16x8_extadd_pairwise_i8x16_s:
 .functype simd_i16x8_extadd_pairwise_i8x16_s (v128) -> (v128)
 local.get 0
 i16x8.extadd_pairwise_i8x16_s
 end_function

.section .text.simd_i16x8_extadd_pairwise_i8x16_u,"",@
.globl simd_i16x8_extadd_pairwise_i8x16_u
simd_i16x8_extadd_pairwise_i8x16_u:
 .functype simd_i16x8_extadd_pairwise_i8x16_u (v128) -> (v128)
 local.get 0
 i16x8.extadd_pairwise_i8x16_u
 end_function

.section .text.simd_i32x4_extadd_pairwise_i16x8_s,"",@
.globl simd_i32x4_extadd_pairwise_i16x8_s
simd_i32x4_extadd_pairwise_i16x8_s:
 .functype simd_i32x4_extadd_pairwise_i16x8_s (v128) -> (v128)
 local.get 0
 i32x4.extadd_pairwise_i16x8_s
 end_function

.section .text.simd_i32x4_extadd_pairwise_i16x8_u,"",@
.globl simd_i32x4_extadd_pairwise_i16x8_u
simd_i32x4_extadd_pairwise_i16x8_u:
 .functype simd_i32x4_extadd_pairwise_i16x8_u (v128) -> (v128)
 local.get 0
 i32x4.extadd_pairwise_i16x8_u
 end_function

.section .text.simd_i16x8_abs,"",@
.globl simd_i16x8_abs
simd_i16x8_abs:
 .functype simd_i16x8_abs (v128) -> (v128)
 local.get 0
 i16x8.abs
 end_function

.section .text.simd_i16x8_neg,"",@
.globl simd_i16x8_neg
simd_i16x8_neg:
 .functype simd_i16x8_neg (v128) -> (v128)
 local.get 0
 i16x8.neg
 end_function

.section .text.simd_i16x8_q15mulr_sat_s,"",@
.globl simd_i16x8_q15mulr_sat_s
simd_i16x8_q15mulr_sat_s:
 .functype simd_i16x8_q15mulr_sat_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i16x8.q15mulr_sat_s
 end_function

.section .text.simd_i16x8_all_true,"",@
.globl simd_i16x8_all_true
simd_i16x8_all_true:
 .functype simd_i16x8_all_true (v128) -> (i32)
 local.get 0
 i16x8.all_true
 end_function

.section .text.simd_i16x8_bitmask,"",@
.globl simd_i16x8_bitmask
simd_i16x8_bitmask:
 .functype simd_i16x8_bitmask (v128) -> (i32)
 local.get 0
 i16x8.bitmask
 end_function

.section .text.simd_i16x8_narrow_i32x4_s,"",@
.globl simd_i16x8_narrow_i32x4_s
simd_i16x8_narrow_i32x4_s:
 .functype simd_i16x8_narrow_i32x4_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i16x8.narrow_i32x4_s
 end_function

.section .text.simd_i16x8_narrow_i32x4_u,"",@
.globl simd_i16x8_narrow_i32x4_u
simd_i16x8_narrow_i32x4_u:
 .functype simd_i16x8_narrow_i32x4_u (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i16x8.narrow_i32x4_u
 end_function

.section .text.simd_i16x8_extend_low_i8x16_s,"",@
.globl simd_i16x8_extend_low_i8x16_s
simd_i16x8_extend_low_i8x16_s:
 .functype simd_i16x8_extend_low_i8x16_s (v128) -> (v128)
 local.get 0
 i16x8.extend_low_i8x16_s
 end_function

.section .text.simd_i16x8_extend_high_i8x16_s,"",@
.globl simd_i16x8_extend_high_i8x16_s
simd_i16x8_extend_high_i8x16_s:
 .functype simd_i16x8_extend_high_i8x16_s (v128) -> (v128)
 local.get 0
 i16x8.extend_high_i8x16_s
 end_function

.section .text.simd_i16x8_extend_low_i8x16_u,"",@
.globl simd_i16x8_extend_low_i8x16_u
simd_i16x8_extend_low_i8x16_u:
 .functype simd_i16x8_extend_low_i8x16_u (v128) -> (v128)
 local.get 0
 i16x8.extend_low_i8x16_u
 end_function

.section .text.simd_i16x8_extend_high_i8x16_u,"",@
.globl simd_i16x8_extend_high_i8x16_u
simd_i16x8_extend_high_i8x16_u:
 .functype simd_i16x8_extend_high_i8x16_u (v128) -> (v128)
 local.get 0
 i16x8.extend_high_i8x16_u
 end_function

.section .text.simd_i16x8_shl,"",@
.globl simd_i16x8_shl
simd_i16x8_shl:
 .functype simd_i16x8_shl (v128, i32) -> (v128)
 local.get 0
 local.get 1
 i16x8.shl
 end_function

.section .text.simd_i16x8_shr_s,"",@
.globl simd_i16x8_shr_s
simd_i16x8_shr_s:
 .functype simd_i16x8_shr_s (v128, i32) -> (v128)
 local.get 0
 local.get 1
 i16x8.shr_s
 end_function

.section .text.simd_i16x8_shr_u,"",@
.globl simd_i16x8_shr_u
simd_i16x8_shr_u:
 .functype simd_i16x8_shr_u (v128, i32) -> (v128)
 local.get 0
 local.get 1
 i16x8.shr_u
 end_function

.section .text.simd_i16x8_add,"",@
.globl simd_i16x8_add
simd_i16x8_add:
 .functype simd_i16x8_add (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i16x8.add
 end_function

.section .text.simd_i16x8_add_sat_s,"",@
.globl simd_i16x8_add_sat_s
simd_i16x8_add_sat_s:
 .functype simd_i16x8_add_sat_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i16x8.add_sat_s
 end_function

.section .text.simd_i16x8_add_sat_u,"",@
.globl simd_i16x8_add_sat_u
simd_i16x8_add_sat_u:
 .functype simd_i16x8_add_sat_u (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i16x8.add_sat_u
 end_function

.section .text.simd_i16x8_sub,"",@
.globl simd_i16x8_sub
simd_i16x8_sub:
 .functype simd_i16x8_sub (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i16x8.sub
 end_function

.section .text.simd_i16x8_sub_sat_s,"",@
.globl simd_i16x8_sub_sat_s
simd_i16x8_sub_sat_s:
 .functype simd_i16x8_sub_sat_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i16x8.sub_sat_s
 end_function

.section .text.simd_i16x8_sub_sat_u,"",@
.globl simd_i16x8_sub_sat_u
simd_i16x8_sub_sat_u:
 .functype simd_i16x8_sub_sat_u (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i16x8.sub_sat_u
 end_function

.section .text.simd_f64x2_nearest,"",@
.globl simd_f64x2_nearest
simd_f64x2_nearest:
 .functype simd_f64x2_nearest (v128) -> (v128)
 local.get 0
 f64x2.nearest
 end_function

.section .text.simd_i16x8_mul,"",@
.globl simd_i16x8_mul
simd_i16x8_mul:
 .functype simd_i16x8_mul (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i16x8.mul
 end_function

.section .text.simd_i16x8_min_s,"",@
.globl simd_i16x8_min_s
simd_i16x8_min_s:
 .functype simd_i16x8_min_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i16x8.min_s
 end_function

.section .text.simd_i16x8_min_u,"",@
.globl simd_i16x8_min_u
simd_i16x8_min_u:
 .functype simd_i16x8_min_u (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i16x8.min_u
 end_function

.section .text.simd_i16x8_max_s,"",@
.globl simd_i16x8_max_s
simd_i16x8_max_s:
 .functype simd_i16x8_max_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i16x8.max_s
 end_function

.section .text.simd_i16x8_max_u,"",@
.globl simd_i16x8_max_u
simd_i16x8_max_u:
 .functype simd_i16x8_max_u (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i16x8.max_u
 end_function

.section .text.simd_i16x8_avgr_u,"",@
.globl simd_i16x8_avgr_u
simd_i16x8_avgr_u:
 .functype simd_i16x8_avgr_u (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i16x8.avgr_u
 end_function

.section .text.simd_i16x8_extmul_low_i8x16_s,"",@
.globl simd_i16x8_extmul_low_i8x16_s
simd_i16x8_extmul_low_i8x16_s:
 .functype simd_i16x8_extmul_low_i8x16_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i16x8.extmul_low_i8x16_s
 end_function

.section .text.simd_i16x8_extmul_high_i8x16_s,"",@
.globl simd_i16x8_extmul_high_i8x16_s
simd_i16x8_extmul_high_i8x16_s:
 .functype simd_i16x8_extmul_high_i8x16_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i16x8.extmul_high_i8x16_s
 end_function

.section .text.simd_i16x8_extmul_low_i8x16_u,"",@
.globl simd_i16x8_extmul_low_i8x16_u
simd_i16x8_extmul_low_i8x16_u:
 .functype simd_i16x8_extmul_low_i8x16_u (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i16x8.extmul_low_i8x16_u
 end_function

.section .text.simd_i16x8_extmul_high_i8x16_u,"",@
.globl simd_i16x8_extmul_high_i8x16_u
simd_i16x8_extmul_high_i8x16_u:
 .functype simd_i16x8_extmul_high_i8x16_u (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i16x8.extmul_high_i8x16_u
 end_function

.section .text.simd_i32x4_abs,"",@
.globl simd_i32x4_abs
simd_i32x4_abs:
 .functype simd_i32x4_abs (v128) -> (v128)
 local.get 0
 i32x4.abs
 end_function

.section .text.simd_i32x4_neg,"",@
.globl simd_i32x4_neg
simd_i32x4_neg:
 .functype simd_i32x4_neg (v128) -> (v128)
 local.get 0
 i32x4.neg
 end_function

.section .text.simd_i32x4_all_true,"",@
.globl simd_i32x4_all_true
simd_i32x4_all_true:
 .functype simd_i32x4_all_true (v128) -> (i32)
 local.get 0
 i32x4.all_true
 end_function

.section .text.simd_i32x4_bitmask,"",@
.globl simd_i32x4_bitmask
simd_i32x4_bitmask:
 .functype simd_i32x4_bitmask (v128) -> (i32)
 local.get 0
 i32x4.bitmask
 end_function

.section .text.simd_i32x4_extend_low_i16x8_s,"",@
.globl simd_i32x4_extend_low_i16x8_s
simd_i32x4_extend_low_i16x8_s:
 .functype simd_i32x4_extend_low_i16x8_s (v128) -> (v128)
 local.get 0
 i32x4.extend_low_i16x8_s
 end_function

.section .text.simd_i32x4_extend_high_i16x8_s,"",@
.globl simd_i32x4_extend_high_i16x8_s
simd_i32x4_extend_high_i16x8_s:
 .functype simd_i32x4_extend_high_i16x8_s (v128) -> (v128)
 local.get 0
 i32x4.extend_high_i16x8_s
 end_function

.section .text.simd_i32x4_extend_low_i16x8_u,"",@
.globl simd_i32x4_extend_low_i16x8_u
simd_i32x4_extend_low_i16x8_u:
 .functype simd_i32x4_extend_low_i16x8_u (v128) -> (v128)
 local.get 0
 i32x4.extend_low_i16x8_u
 end_function

.section .text.simd_i32x4_extend_high_i16x8_u,"",@
.globl simd_i32x4_extend_high_i16x8_u
simd_i32x4_extend_high_i16x8_u:
 .functype simd_i32x4_extend_high_i16x8_u (v128) -> (v128)
 local.get 0
 i32x4.extend_high_i16x8_u
 end_function

.section .text.simd_i32x4_shl,"",@
.globl simd_i32x4_shl
simd_i32x4_shl:
 .functype simd_i32x4_shl (v128, i32) -> (v128)
 local.get 0
 local.get 1
 i32x4.shl
 end_function

.section .text.simd_i32x4_shr_s,"",@
.globl simd_i32x4_shr_s
simd_i32x4_shr_s:
 .functype simd_i32x4_shr_s (v128, i32) -> (v128)
 local.get 0
 local.get 1
 i32x4.shr_s
 end_function

.section .text.simd_i32x4_shr_u,"",@
.globl simd_i32x4_shr_u
simd_i32x4_shr_u:
 .functype simd_i32x4_shr_u (v128, i32) -> (v128)
 local.get 0
 local.get 1
 i32x4.shr_u
 end_function

.section .text.simd_i32x4_add,"",@
.globl simd_i32x4_add
simd_i32x4_add:
 .functype simd_i32x4_add (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i32x4.add
 end_function

.section .text.simd_i32x4_sub,"",@
.globl simd_i32x4_sub
simd_i32x4_sub:
 .functype simd_i32x4_sub (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i32x4.sub
 end_function

.section .text.simd_i32x4_mul,"",@
.globl simd_i32x4_mul
simd_i32x4_mul:
 .functype simd_i32x4_mul (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i32x4.mul
 end_function

.section .text.simd_i32x4_min_s,"",@
.globl simd_i32x4_min_s
simd_i32x4_min_s:
 .functype simd_i32x4_min_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i32x4.min_s
 end_function

.section .text.simd_i32x4_min_u,"",@
.globl simd_i32x4_min_u
simd_i32x4_min_u:
 .functype simd_i32x4_min_u (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i32x4.min_u
 end_function

.section .text.simd_i32x4_max_s,"",@
.globl simd_i32x4_max_s
simd_i32x4_max_s:
 .functype simd_i32x4_max_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i32x4.max_s
 end_function

.section .text.simd_i32x4_max_u,"",@
.globl simd_i32x4_max_u
simd_i32x4_max_u:
 .functype simd_i32x4_max_u (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i32x4.max_u
 end_function

.section .text.simd_i32x4_dot_i16x8_s,"",@
.globl simd_i32x4_dot_i16x8_s
simd_i32x4_dot_i16x8_s:
 .functype simd_i32x4_dot_i16x8_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i32x4.dot_i16x8_s
 end_function

.section .text.simd_i32x4_extmul_low_i16x8_s,"",@
.globl simd_i32x4_extmul_low_i16x8_s
simd_i32x4_extmul_low_i16x8_s:
 .functype simd_i32x4_extmul_low_i16x8_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i32x4.extmul_low_i16x8_s
 end_function

.section .text.simd_i32x4_extmul_high_i16x8_s,"",@
.globl simd_i32x4_extmul_high_i16x8_s
simd_i32x4_extmul_high_i16x8_s:
 .functype simd_i32x4_extmul_high_i16x8_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i32x4.extmul_high_i16x8_s
 end_function

.section .text.simd_i32x4_extmul_low_i16x8_u,"",@
.globl simd_i32x4_extmul_low_i16x8_u
simd_i32x4_extmul_low_i16x8_u:
 .functype simd_i32x4_extmul_low_i16x8_u (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i32x4.extmul_low_i16x8_u
 end_function

.section .text.simd_i32x4_extmul_high_i16x8_u,"",@
.globl simd_i32x4_extmul_high_i16x8_u
simd_i32x4_extmul_high_i16x8_u:
 .functype simd_i32x4_extmul_high_i16x8_u (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i32x4.extmul_high_i16x8_u
 end_function

.section .text.simd_i64x2_abs,"",@
.globl simd_i64x2_abs
simd_i64x2_abs:
 .functype simd_i64x2_abs (v128) -> (v128)
 local.get 0
 i64x2.abs
 end_function

.section .text.simd_i64x2_neg,"",@
.globl simd_i64x2_neg
simd_i64x2_neg:
 .functype simd_i64x2_neg (v128) -> (v128)
 local.get 0
 i64x2.neg
 end_function

.section .text.simd_i64x2_all_true,"",@
.globl simd_i64x2_all_true
simd_i64x2_all_true:
 .functype simd_i64x2_all_true (v128) -> (i32)
 local.get 0
 i64x2.all_true
 end_function

.section .text.simd_i64x2_bitmask,"",@
.globl simd_i64x2_bitmask
simd_i64x2_bitmask:
 .functype simd_i64x2_bitmask (v128) -> (i32)
 local.get 0
 i64x2.bitmask
 end_function

.section .text.simd_i64x2_extend_low_i32x4_s,"",@
.globl simd_i64x2_extend_low_i32x4_s
simd_i64x2_extend_low_i32x4_s:
 .functype simd_i64x2_extend_low_i32x4_s (v128) -> (v128)
 local.get 0
 i64x2.extend_low_i32x4_s
 end_function

.section .text.simd_i64x2_extend_high_i32x4_s,"",@
.globl simd_i64x2_extend_high_i32x4_s
simd_i64x2_extend_high_i32x4_s:
 .functype simd_i64x2_extend_high_i32x4_s (v128) -> (v128)
 local.get 0
 i64x2.extend_high_i32x4_s
 end_function

.section .text.simd_i64x2_extend_low_i32x4_u,"",@
.globl simd_i64x2_extend_low_i32x4_u
simd_i64x2_extend_low_i32x4_u:
 .functype simd_i64x2_extend_low_i32x4_u (v128) -> (v128)
 local.get 0
 i64x2.extend_low_i32x4_u
 end_function

.section .text.simd_i64x2_extend_high_i32x4_u,"",@
.globl simd_i64x2_extend_high_i32x4_u
simd_i64x2_extend_high_i32x4_u:
 .functype simd_i64x2_extend_high_i32x4_u (v128) -> (v128)
 local.get 0
 i64x2.extend_high_i32x4_u
 end_function

.section .text.simd_i64x2_shl,"",@
.globl simd_i64x2_shl
simd_i64x2_shl:
 .functype simd_i64x2_shl (v128, i32) -> (v128)
 local.get 0
 local.get 1
 i64x2.shl
 end_function

.section .text.simd_i64x2_shr_s,"",@
.globl simd_i64x2_shr_s
simd_i64x2_shr_s:
 .functype simd_i64x2_shr_s (v128, i32) -> (v128)
 local.get 0
 local.get 1
 i64x2.shr_s
 end_function

.section .text.simd_i64x2_shr_u,"",@
.globl simd_i64x2_shr_u
simd_i64x2_shr_u:
 .functype simd_i64x2_shr_u (v128, i32) -> (v128)
 local.get 0
 local.get 1
 i64x2.shr_u
 end_function

.section .text.simd_i64x2_add,"",@
.globl simd_i64x2_add
simd_i64x2_add:
 .functype simd_i64x2_add (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i64x2.add
 end_function

.section .text.simd_i64x2_sub,"",@
.globl simd_i64x2_sub
simd_i64x2_sub:
 .functype simd_i64x2_sub (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i64x2.sub
 end_function

.section .text.simd_i64x2_mul,"",@
.globl simd_i64x2_mul
simd_i64x2_mul:
 .functype simd_i64x2_mul (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i64x2.mul
 end_function

.section .text.simd_i64x2_eq,"",@
.globl simd_i64x2_eq
simd_i64x2_eq:
 .functype simd_i64x2_eq (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i64x2.eq
 end_function

.section .text.simd_i64x2_ne,"",@
.globl simd_i64x2_ne
simd_i64x2_ne:
 .functype simd_i64x2_ne (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i64x2.ne
 end_function

.section .text.simd_i64x2_lt_s,"",@
.globl simd_i64x2_lt_s
simd_i64x2_lt_s:
 .functype simd_i64x2_lt_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i64x2.lt_s
 end_function

.section .text.simd_i64x2_gt_s,"",@
.globl simd_i64x2_gt_s
simd_i64x2_gt_s:
 .functype simd_i64x2_gt_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i64x2.gt_s
 end_function

.section .text.simd_i64x2_le_s,"",@
.globl simd_i64x2_le_s
simd_i64x2_le_s:
 .functype simd_i64x2_le_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i64x2.le_s
 end_function

.section .text.simd_i64x2_ge_s,"",@
.globl simd_i64x2_ge_s
simd_i64x2_ge_s:
 .functype simd_i64x2_ge_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i64x2.ge_s
 end_function

.section .text.simd_i64x2_extmul_low_i32x4_s,"",@
.globl simd_i64x2_extmul_low_i32x4_s
simd_i64x2_extmul_low_i32x4_s:
 .functype simd_i64x2_extmul_low_i32x4_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i64x2.extmul_low_i32x4_s
 end_function

.section .text.simd_i64x2_extmul_high_i32x4_s,"",@
.globl simd_i64x2_extmul_high_i32x4_s
simd_i64x2_extmul_high_i32x4_s:
 .functype simd_i64x2_extmul_high_i32x4_s (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i64x2.extmul_high_i32x4_s
 end_function

.section .text.simd_i64x2_extmul_low_i32x4_u,"",@
.globl simd_i64x2_extmul_low_i32x4_u
simd_i64x2_extmul_low_i32x4_u:
 .functype simd_i64x2_extmul_low_i32x4_u (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i64x2.extmul_low_i32x4_u
 end_function

.section .text.simd_i64x2_extmul_high_i32x4_u,"",@
.globl simd_i64x2_extmul_high_i32x4_u
simd_i64x2_extmul_high_i32x4_u:
 .functype simd_i64x2_extmul_high_i32x4_u (v128, v128) -> (v128)
 local.get 0
 local.get 1
 i64x2.extmul_high_i32x4_u
 end_function

.section .text.simd_f32x4_abs,"",@
.globl simd_f32x4_abs
simd_f32x4_abs:
 .functype simd_f32x4_abs (v128) -> (v128)
 local.get 0
 f32x4.abs
 end_function

.section .text.simd_f32x4_neg,"",@
.globl simd_f32x4_neg
simd_f32x4_neg:
 .functype simd_f32x4_neg (v128) -> (v128)
 local.get 0
 f32x4.neg
 end_function

.section .text.simd_f32x4_sqrt,"",@
.globl simd_f32x4_sqrt
simd_f32x4_sqrt:
 .functype simd_f32x4_sqrt (v128) -> (v128)
 local.get 0
 f32x4.sqrt
 end_function

.section .text.simd_f32x4_add,"",@
.globl simd_f32x4_add
simd_f32x4_add:
 .functype simd_f32x4_add (v128, v128) -> (v128)
 local.get 0
 local.get 1
 f32x4.add
 end_function

.section .text.simd_f32x4_sub,"",@
.globl simd_f32x4_sub
simd_f32x4_sub:
 .functype simd_f32x4_sub (v128, v128) -> (v128)
 local.get 0
 local.get 1
 f32x4.sub
 end_function

.section .text.simd_f32x4_mul,"",@
.globl simd_f32x4_mul
simd_f32x4_mul:
 .functype simd_f32x4_mul (v128, v128) -> (v128)
 local.get 0
 local.get 1
 f32x4.mul
 end_function

.section .text.simd_f32x4_div,"",@
.globl simd_f32x4_div
simd_f32x4_div:
 .functype simd_f32x4_div (v128, v128) -> (v128)
 local.get 0
 local.get 1
 f32x4.div
 end_function

.section .text.simd_f32x4_min,"",@
.globl simd_f32x4_min
simd_f32x4_min:
 .functype simd_f32x4_min (v128, v128) -> (v128)
 local.get 0
 local.get 1
 f32x4.min
 end_function

.section .text.simd_f32x4_max,"",@
.globl simd_f32x4_max
simd_f32x4_max:
 .functype simd_f32x4_max (v128, v128) -> (v128)
 local.get 0
 local.get 1
 f32x4.max
 end_function

.section .text.simd_f32x4_pmin,"",@
.globl simd_f32x4_pmin
simd_f32x4_pmin:
 .functype simd_f32x4_pmin (v128, v128) -> (v128)
 local.get 0
 local.get 1
 f32x4.pmin
 end_function

.section .text.simd_f32x4_pmax,"",@
.globl simd_f32x4_pmax
simd_f32x4_pmax:
 .functype simd_f32x4_pmax (v128, v128) -> (v128)
 local.get 0
 local.get 1
 f32x4.pmax
 end_function

.section .text.simd_f64x2_abs,"",@
.globl simd_f64x2_abs
simd_f64x2_abs:
 .functype simd_f64x2_abs (v128) -> (v128)
 local.get 0
 f64x2.abs
 end_function

.section .text.simd_f64x2_neg,"",@
.globl simd_f64x2_neg
simd_f64x2_neg:
 .functype simd_f64x2_neg (v128) -> (v128)
 local.get 0
 f64x2.neg
 end_function

.section .text.simd_f64x2_sqrt,"",@
.globl simd_f64x2_sqrt
simd_f64x2_sqrt:
 .functype simd_f64x2_sqrt (v128) -> (v128)
 local.get 0
 f64x2.sqrt
 end_function

.section .text.simd_f64x2_add,"",@
.globl simd_f64x2_add
simd_f64x2_add:
 .functype simd_f64x2_add (v128, v128) -> (v128)
 local.get 0
 local.get 1
 f64x2.add
 end_function

.section .text.simd_f64x2_sub,"",@
.globl simd_f64x2_sub
simd_f64x2_sub:
 .functype simd_f64x2_sub (v128, v128) -> (v128)
 local.get 0
 local.get 1
 f64x2.sub
 end_function

.section .text.simd_f64x2_mul,"",@
.globl simd_f64x2_mul
simd_f64x2_mul:
 .functype simd_f64x2_mul (v128, v128) -> (v128)
 local.get 0
 local.get 1
 f64x2.mul
 end_function

.section .text.simd_f64x2_div,"",@
.globl simd_f64x2_div
simd_f64x2_div:
 .functype simd_f64x2_div (v128, v128) -> (v128)
 local.get 0
 local.get 1
 f64x2.div
 end_function

.section .text.simd_f64x2_min,"",@
.globl simd_f64x2_min
simd_f64x2_min:
 .functype simd_f64x2_min (v128, v128) -> (v128)
 local.get 0
 local.get 1
 f64x2.min
 end_function

.section .text.simd_f64x2_max,"",@
.globl simd_f64x2_max
simd_f64x2_max:
 .functype simd_f64x2_max (v128, v128) -> (v128)
 local.get 0
 local.get 1
 f64x2.max
 end_function

.section .text.simd_f64x2_pmin,"",@
.globl simd_f64x2_pmin
simd_f64x2_pmin:
 .functype simd_f64x2_pmin (v128, v128) -> (v128)
 local.get 0
 local.get 1
 f64x2.pmin
 end_function

.section .text.simd_f64x2_pmax,"",@
.globl simd_f64x2_pmax
simd_f64x2_pmax:
 .functype simd_f64x2_pmax (v128, v128) -> (v128)
 local.get 0
 local.get 1
 f64x2.pmax
 end_function

.section .text.simd_i32x4_trunc_sat_f32x4_s,"",@
.globl simd_i32x4_trunc_sat_f32x4_s
simd_i32x4_trunc_sat_f32x4_s:
 .functype simd_i32x4_trunc_sat_f32x4_s (v128) -> (v128)
 local.get 0
 i32x4.trunc_sat_f32x4_s
 end_function

.section .text.simd_i32x4_trunc_sat_f32x4_u,"",@
.globl simd_i32x4_trunc_sat_f32x4_u
simd_i32x4_trunc_sat_f32x4_u:
 .functype simd_i32x4_trunc_sat_f32x4_u (v128) -> (v128)
 local.get 0
 i32x4.trunc_sat_f32x4_u
 end_function

.section .text.simd_f32x4_convert_i32x4_s,"",@
.globl simd_f32x4_convert_i32x4_s
simd_f32x4_convert_i32x4_s:
 .functype simd_f32x4_convert_i32x4_s (v128) -> (v128)
 local.get 0
 f32x4.convert_i32x4_s
 end_function

.section .text.simd_f32x4_convert_i32x4_u,"",@
.globl simd_f32x4_convert_i32x4_u
simd_f32x4_convert_i32x4_u:
 .functype simd_f32x4_convert_i32x4_u (v128) -> (v128)
 local.get 0
 f32x4.convert_i32x4_u
 end_function

.section .text.simd_i32x4_trunc_sat_f64x2_s_zero,"",@
.globl simd_i32x4_trunc_sat_f64x2_s_zero
simd_i32x4_trunc_sat_f64x2_s_zero:
 .functype simd_i32x4_trunc_sat_f64x2_s_zero (v128) -> (v128)
 local.get 0
 i32x4.trunc_sat_f64x2_s_zero
 end_function

.section .text.simd_i32x4_trunc_sat_f64x2_u_zero,"",@
.globl simd_i32x4_trunc_sat_f64x2_u_zero
simd_i32x4_trunc_sat_f64x2_u_zero:
 .functype simd_i32x4_trunc_sat_f64x2_u_zero (v128) -> (v128)
 local.get 0
 i32x4.trunc_sat_f64x2_u_zero
 end_function

.section .text.simd_f64x2_convert_low_i32x4_s,"",@
.globl simd_f64x2_convert_low_i32x4_s
simd_f64x2_convert_low_i32x4_s:
 .functype simd_f64x2_convert_low_i32x4_s (v128) -> (v128)
 local.get 0
 f64x2.convert_low_i32x4_s
 end_function

.section .text.simd_f64x2_convert_low_i32x4_u,"",@
.globl simd_f64x2_convert_low_i32x4_u
simd_f64x2_convert_low_i32x4_u:
 .functype simd_f64x2_convert_low_i32x4_u (v128) -> (v128)
 local.get 0
 f64x2.convert_low_i32x4_u
 end_function

.section .text.simd_reference,"",@
.globl simd_reference
simd_reference:
 .functype simd_reference () -> (i32)
 i32.const 32
 i32.const 17
 call simd_i32x4_splat
 call simd_v128_store
 i32.const 32
 call simd_v128_load
 call simd_i32x4_extract_lane
 end_function

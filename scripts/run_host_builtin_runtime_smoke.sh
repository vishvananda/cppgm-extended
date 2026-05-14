#!/bin/bash
set -eu

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
obj_root="${1:-${CPPGM_OBJECT_ROOT:-${repo_root}/obj}}"
host_cxx="${CPPGM_HOST_CXX:-${CXX:-/usr/local/opt/llvm/bin/clang++}}"
runtime_obj="${obj_root}/release/host_builtin_runtime.o"
fallback_runtime_obj="${repo_root}/obj/release/host_builtin_runtime.o"

if [ ! -f "${runtime_obj}" ] && [ -f "${fallback_runtime_obj}" ]; then
  runtime_obj="${fallback_runtime_obj}"
fi

if [ ! -f "${runtime_obj}" ]; then
  echo "missing host builtin runtime object: ${runtime_obj}" >&2
  exit 1
fi

tmpdir="$(mktemp -d "${TMPDIR:-/tmp}/cppgm-host-builtin-runtime-smoke.XXXXXX")"
trap 'rm -rf "${tmpdir}"' EXIT

src="${tmpdir}/host_builtin_runtime_smoke.cpp"
exe="${tmpdir}/host_builtin_runtime_smoke"

cat > "${src}" <<'EOF'
#include <cmath>
#include <cstddef>
#include <cstring>
#include <ios>
#include <iterator>
#include <locale>
#include <sstream>
#include <streambuf>

extern "C" void cppgm_host_num_put_char_put_bool(void *, const void *, const void *, void *, unsigned char, bool);
extern "C" void cppgm_host_num_put_char_put_long(void *, const void *, const void *, void *, unsigned char, long);
extern "C" void cppgm_host_num_put_char_put_long_long(void *, const void *, const void *, void *, unsigned char, long long);
extern "C" void cppgm_host_num_put_char_put_unsigned_long(void *, const void *, const void *, void *, unsigned char, unsigned long);
extern "C" void cppgm_host_num_put_char_put_unsigned_long_long(void *, const void *, const void *, void *, unsigned char, unsigned long long);
extern "C" void cppgm_host_num_put_char_put_double(void *, const void *, const void *, void *, unsigned char, double);
extern "C" void cppgm_host_num_put_char_put_long_double(void *, const void *, const void *, void *, unsigned char, long double);
extern "C" void cppgm_host_num_put_char_put_ptr(void *, const void *, const void *, void *, unsigned char, const void *);

extern "C" double cppgm_builtin_ceil(double);
extern "C" float cppgm_builtin_ceilf(float);
extern "C" long double cppgm_builtin_ceill(long double);
extern "C" double cppgm_builtin_fabs(double);
extern "C" float cppgm_builtin_fabsf(float);
extern "C" long double cppgm_builtin_fabsl(long double);
extern "C" double cppgm_builtin_inf();
extern "C" float cppgm_builtin_inff();
extern "C" long double cppgm_builtin_infl();
extern "C" bool cppgm_builtin_is_constant_evaluated();
extern "C" bool cppgm_builtin_isfinite(double);
extern "C" bool cppgm_builtin_isfinitef(float);
extern "C" bool cppgm_builtin_isfinitel(long double);
extern "C" bool cppgm_builtin_isinf(double);
extern "C" bool cppgm_builtin_isinff(float);
extern "C" bool cppgm_builtin_isinfl(long double);
extern "C" bool cppgm_builtin_isnan(double);
extern "C" bool cppgm_builtin_isnanf(float);
extern "C" bool cppgm_builtin_isnanl(long double);
extern "C" bool cppgm_builtin_isnormal(double);
extern "C" bool cppgm_builtin_isnormalf(float);
extern "C" bool cppgm_builtin_isnormall(long double);
extern "C" void * cppgm_builtin_memchr(const void *, int, unsigned long);
extern "C" int cppgm_builtin_memcmp(const void *, const void *, unsigned long);
extern "C" void * cppgm_builtin_memcpy(void *, const void *, unsigned long);
extern "C" void * cppgm_builtin_memmove(void *, const void *, unsigned long);
extern "C" long cppgm_builtin_expect(long, long);
extern "C" int cppgm_builtin_strcmp(const char *, const char *);
extern "C" unsigned long cppgm_builtin_strlen(const char *);
extern "C" void * cppgm_builtin_operator_new(unsigned long);
extern "C" void * cppgm_builtin_operator_new_aligned(unsigned long, unsigned long);
extern "C" void * cppgm_builtin_operator_new_array(unsigned long);
extern "C" void * cppgm_builtin_operator_new_array_aligned(unsigned long, unsigned long);
extern "C" void cppgm_builtin_operator_delete(void *);
extern "C" void cppgm_builtin_operator_delete_sized(void *, unsigned long);
extern "C" void cppgm_builtin_operator_delete_aligned(void *, unsigned long);
extern "C" void cppgm_builtin_operator_delete_sized_aligned(void *, unsigned long, unsigned long);
extern "C" void cppgm_builtin_operator_delete_array(void *);
extern "C" void cppgm_builtin_operator_delete_array_sized(void *, unsigned long);
extern "C" void cppgm_builtin_operator_delete_array_aligned(void *, unsigned long);
extern "C" void cppgm_builtin_operator_delete_array_sized_aligned(void *, unsigned long, unsigned long);

template<class Value, class Fn>
bool check_num_put(Fn fn, Value value)
{
  std::ostringstream out;
  std::ostreambuf_iterator<char> iter(out);
  std::ostreambuf_iterator<char> ret(static_cast<std::streambuf *>(0));
  const std::num_put<char> & facet = std::use_facet<std::num_put<char> >(std::locale::classic());
  std::ios_base & ios = out;
  fn(&ret, &facet, &iter, &ios, static_cast<unsigned char>(' '), value);
  return !out.str().empty();
}

int main()
{
  if(cppgm_builtin_ceil(1.2) != 2.0 ||
     cppgm_builtin_ceilf(1.2f) != 2.0f ||
     cppgm_builtin_ceill(1.2L) != 2.0L) return 1;
  if(cppgm_builtin_fabs(-3.0) != 3.0 ||
     cppgm_builtin_fabsf(-3.0f) != 3.0f ||
     cppgm_builtin_fabsl(-3.0L) != 3.0L) return 2;

  const double inf = cppgm_builtin_inf();
  const float inff = cppgm_builtin_inff();
  const long double infl = cppgm_builtin_infl();
  if(cppgm_builtin_is_constant_evaluated()) return 3;
  if(!cppgm_builtin_isinf(inf) ||
     !cppgm_builtin_isinff(inff) ||
     !cppgm_builtin_isinfl(infl)) return 4;
  if(!cppgm_builtin_isfinite(1.0) ||
     !cppgm_builtin_isfinitef(1.0f) ||
     !cppgm_builtin_isfinitel(1.0L)) return 5;
  if(!cppgm_builtin_isnormal(1.0) ||
     !cppgm_builtin_isnormalf(1.0f) ||
     !cppgm_builtin_isnormall(1.0L)) return 6;
  if(!cppgm_builtin_isnan(inf / inf) ||
     !cppgm_builtin_isnanf(inff / inff) ||
     !cppgm_builtin_isnanl(infl / infl)) return 7;

  char buf[16] = {};
  const char src[] = "abcdef";
  if(cppgm_builtin_memcpy(buf, src, 7) != buf) return 8;
  if(cppgm_builtin_memcmp(buf, src, 7) != 0) return 9;
  if(cppgm_builtin_memchr(buf, 'd', 7) != buf + 3) return 10;
  if(cppgm_builtin_memmove(buf + 1, buf, 6) != buf + 1) return 11;
  if(cppgm_builtin_strcmp("abc", "abc") != 0) return 12;
  if(cppgm_builtin_strlen("abc") != 3) return 13;
  if(cppgm_builtin_expect(4, 9) != 4) return 14;

  void * p0 = cppgm_builtin_operator_new(0);
  void * p1 = cppgm_builtin_operator_new_aligned(1, 3);
  void * p2 = cppgm_builtin_operator_new_array(2);
  void * p3 = cppgm_builtin_operator_new_array_aligned(3, 64);
  cppgm_builtin_operator_delete(p0);
  cppgm_builtin_operator_delete_sized(p2, 2);
  cppgm_builtin_operator_delete_aligned(p1, 3);
  cppgm_builtin_operator_delete_array_aligned(p3, 64);

  void * p4 = cppgm_builtin_operator_new(4);
  void * p5 = cppgm_builtin_operator_new_aligned(5, 32);
  void * p6 = cppgm_builtin_operator_new_array(6);
  void * p7 = cppgm_builtin_operator_new_array_aligned(7, 128);
  cppgm_builtin_operator_delete_sized_aligned(p5, 5, 32);
  cppgm_builtin_operator_delete_array(p6);
  cppgm_builtin_operator_delete_array_sized(p4, 4);
  cppgm_builtin_operator_delete_array_sized_aligned(p7, 7, 128);

  if(!check_num_put(cppgm_host_num_put_char_put_bool, true)) return 15;
  if(!check_num_put(cppgm_host_num_put_char_put_long, 12L)) return 16;
  if(!check_num_put(cppgm_host_num_put_char_put_long_long, 13LL)) return 17;
  if(!check_num_put(cppgm_host_num_put_char_put_unsigned_long, 14UL)) return 18;
  if(!check_num_put(cppgm_host_num_put_char_put_unsigned_long_long, 15ULL)) return 19;
  if(!check_num_put(cppgm_host_num_put_char_put_double, 1.5)) return 20;
  if(!check_num_put(cppgm_host_num_put_char_put_long_double, 2.5L)) return 21;
  if(!check_num_put(cppgm_host_num_put_char_put_ptr, static_cast<const void *>(&src[0]))) return 22;

  return 0;
}
EOF

"${host_cxx}" -std=gnu++11 -fprofile-instr-generate -fcoverage-mapping \
  -o "${exe}" "${src}" "${runtime_obj}"

"${exe}"

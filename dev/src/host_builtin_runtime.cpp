#include <cmath>
#include <cstdlib>
#include <iterator>
#include <locale>
#include <cstring>
#include <limits>

namespace {

using OStreamBufIteratorChar =
    std::ostreambuf_iterator<char, std::char_traits<char> >;
using NumPutChar =
    std::num_put<char, OStreamBufIteratorChar>;
using OStreamBufIteratorCharBits = void *;

extern "C" int __fpclassifyf(float);
extern "C" int __fpclassifyl(long double);

#if defined(__linux__)
extern "C" int __fpclassify(double);
#else
extern "C" int __fpclassifyd(double);
#endif

int host_fpclassify(float value)
{
  return __fpclassifyf(value);
}

int host_fpclassify(double value)
{
#if defined(__linux__)
  return __fpclassify(value);
#else
  return __fpclassifyd(value);
#endif
}

int host_fpclassify(long double value)
{
  return __fpclassifyl(value);
}

bool classification_isfinite(int classification)
{
  return classification == FP_NORMAL ||
         classification == FP_SUBNORMAL ||
         classification == FP_ZERO;
}

bool classification_isinf(int classification)
{
  return classification == FP_INFINITE;
}

bool classification_isnan(int classification)
{
  return classification == FP_NAN;
}

bool classification_isnormal(int classification)
{
  return classification == FP_NORMAL;
}

unsigned long normalized_allocation_size(unsigned long size)
{
  return size == 0 ? 1 : size;
}

unsigned long normalized_alignment(unsigned long alignment)
{
  const unsigned long min_alignment = sizeof(void *);
  if(alignment < min_alignment) {
    alignment = min_alignment;
  }
  if((alignment & (alignment - 1)) != 0) {
    unsigned long rounded = min_alignment;
    while(rounded < alignment) {
      rounded <<= 1;
    }
    alignment = rounded;
  }
  return alignment;
}

void * host_allocate(unsigned long size)
{
  void * result = std::malloc(normalized_allocation_size(size));
  if(result == nullptr) {
    std::abort();
  }
  return result;
}

void * host_allocate_aligned(unsigned long size, unsigned long alignment)
{
  void * result = nullptr;
  const int rc = ::posix_memalign(&result,
                                  normalized_alignment(alignment),
                                  normalized_allocation_size(size));
  if(rc != 0 || result == nullptr) {
    std::abort();
  }
  return result;
}

OStreamBufIteratorCharBits load_ostreambuf_iterator_char_bits(const void * ptr)
{
  OStreamBufIteratorCharBits bits = nullptr;
  std::memcpy(&bits, ptr, sizeof(bits));
  return bits;
}

void store_ostreambuf_iterator_char_bits(void * ptr, OStreamBufIteratorCharBits bits)
{
  std::memcpy(ptr, &bits, sizeof(bits));
}

template<class Value>
void host_num_put_char_put_vcall(void * ret,
                                 const void * facet_ptr,
                                 const void * iter_ptr,
                                 void * iob_ptr,
                                 unsigned char fill,
                                 Value value,
                                 std::size_t slot_offset)
{
  typedef OStreamBufIteratorCharBits (*VCallFn)(const void *,
                                                OStreamBufIteratorCharBits,
                                                void *,
                                                int,
                                                Value);
  const void * const * address_point =
      *reinterpret_cast<const void * const * const *>(facet_ptr);
  const void * entry =
      *reinterpret_cast<const void * const *>(reinterpret_cast<const char *>(address_point) +
                                              slot_offset);
  VCallFn fn = reinterpret_cast<VCallFn>(const_cast<void *>(entry));
  const OStreamBufIteratorCharBits iter_bits = load_ostreambuf_iterator_char_bits(iter_ptr);
  const OStreamBufIteratorCharBits result =
      fn(facet_ptr, iter_bits, iob_ptr, static_cast<char>(fill), value);
  store_ostreambuf_iterator_char_bits(ret, result);
}

}

extern "C" void cppgm_host_num_put_char_put_bool(void * ret,
                                                 const void * facet_ptr,
                                                 const void * iter_ptr,
                                                 void * iob_ptr,
                                                 unsigned char fill,
                                                 bool value)
{
  host_num_put_char_put_vcall(ret, facet_ptr, iter_ptr, iob_ptr, fill, value, 0x18);
}

extern "C" void cppgm_host_num_put_char_put_long(void * ret,
                                                 const void * facet_ptr,
                                                 const void * iter_ptr,
                                                 void * iob_ptr,
                                                 unsigned char fill,
                                                 long value)
{
  host_num_put_char_put_vcall(ret, facet_ptr, iter_ptr, iob_ptr, fill, value, 0x20);
}

extern "C" void cppgm_host_num_put_char_put_long_long(void * ret,
                                                       const void * facet_ptr,
                                                       const void * iter_ptr,
                                                       void * iob_ptr,
                                                       unsigned char fill,
                                                       long long value)
{
  host_num_put_char_put_vcall(ret, facet_ptr, iter_ptr, iob_ptr, fill, value, 0x28);
}

extern "C" void cppgm_host_num_put_char_put_unsigned_long(void * ret,
                                                          const void * facet_ptr,
                                                          const void * iter_ptr,
                                                          void * iob_ptr,
                                                          unsigned char fill,
                                                          unsigned long value)
{
  host_num_put_char_put_vcall(ret, facet_ptr, iter_ptr, iob_ptr, fill, value, 0x30);
}

extern "C" void cppgm_host_num_put_char_put_unsigned_long_long(
    void * ret,
    const void * facet_ptr,
    const void * iter_ptr,
    void * iob_ptr,
    unsigned char fill,
    unsigned long long value)
{
  host_num_put_char_put_vcall(ret, facet_ptr, iter_ptr, iob_ptr, fill, value, 0x38);
}

extern "C" void cppgm_host_num_put_char_put_double(void * ret,
                                                   const void * facet_ptr,
                                                   const void * iter_ptr,
                                                   void * iob_ptr,
                                                   unsigned char fill,
                                                   double value)
{
  host_num_put_char_put_vcall(ret, facet_ptr, iter_ptr, iob_ptr, fill, value, 0x40);
}

extern "C" void cppgm_host_num_put_char_put_long_double(void * ret,
                                                        const void * facet_ptr,
                                                        const void * iter_ptr,
                                                        void * iob_ptr,
                                                        unsigned char fill,
                                                        long double value)
{
  host_num_put_char_put_vcall(ret, facet_ptr, iter_ptr, iob_ptr, fill, value, 0x48);
}

extern "C" void cppgm_host_num_put_char_put_ptr(void * ret,
                                                const void * facet_ptr,
                                                const void * iter_ptr,
                                                void * iob_ptr,
                                                unsigned char fill,
                                                const void * value)
{
  host_num_put_char_put_vcall(ret, facet_ptr, iter_ptr, iob_ptr, fill, value, 0x50);
}

extern "C" double cppgm_builtin_ceil(double value)
{
  return std::ceil(value);
}

extern "C" float cppgm_builtin_ceilf(float value)
{
  return std::ceil(value);
}

extern "C" long double cppgm_builtin_ceill(long double value)
{
  return std::ceil(value);
}

extern "C" double cppgm_builtin_fabs(double value)
{
  return std::fabs(value);
}

extern "C" float cppgm_builtin_fabsf(float value)
{
  return std::fabs(value);
}

extern "C" long double cppgm_builtin_fabsl(long double value)
{
  return std::fabs(value);
}

extern "C" double cppgm_builtin_inf()
{
  volatile double zero = 0.0;
  return 1.0 / zero;
}

extern "C" float cppgm_builtin_inff()
{
  volatile float zero = 0.0f;
  return 1.0f / zero;
}

extern "C" long double cppgm_builtin_infl()
{
  volatile long double zero = 0.0L;
  return 1.0L / zero;
}

extern "C" double cppgm_builtin_nans(const char *)
{
  return std::numeric_limits<double>::signaling_NaN();
}

extern "C" float cppgm_builtin_nansf(const char *)
{
  return std::numeric_limits<float>::signaling_NaN();
}

extern "C" long double cppgm_builtin_nansl(const char *)
{
  return std::numeric_limits<long double>::signaling_NaN();
}

extern "C" bool cppgm_builtin_is_constant_evaluated()
{
  return false;
}

extern "C" int cppgm_builtin_flt_rounds()
{
  return 1;
}

extern "C" bool cppgm_builtin_isfinite(double value)
{
  return classification_isfinite(host_fpclassify(value));
}

extern "C" bool cppgm_builtin_isfinitef(float value)
{
  return classification_isfinite(host_fpclassify(value));
}

extern "C" bool cppgm_builtin_isfinitel(long double value)
{
  return classification_isfinite(host_fpclassify(value));
}

extern "C" bool cppgm_builtin_isinf(double value)
{
  return classification_isinf(host_fpclassify(value));
}

extern "C" bool cppgm_builtin_isinff(float value)
{
  return classification_isinf(host_fpclassify(value));
}

extern "C" bool cppgm_builtin_isinfl(long double value)
{
  return classification_isinf(host_fpclassify(value));
}

extern "C" bool cppgm_builtin_isnan(double value)
{
  return classification_isnan(host_fpclassify(value));
}

extern "C" bool cppgm_builtin_isnanf(float value)
{
  return classification_isnan(host_fpclassify(value));
}

extern "C" bool cppgm_builtin_isnanl(long double value)
{
  return classification_isnan(host_fpclassify(value));
}

extern "C" bool cppgm_builtin_isnormal(double value)
{
  return classification_isnormal(host_fpclassify(value));
}

extern "C" bool cppgm_builtin_isnormalf(float value)
{
  return classification_isnormal(host_fpclassify(value));
}

extern "C" bool cppgm_builtin_isnormall(long double value)
{
  return classification_isnormal(host_fpclassify(value));
}

extern "C" void * cppgm_builtin_memchr(const void * s, int c, unsigned long n)
{
  const unsigned char * bytes = static_cast<const unsigned char *>(s);
  const unsigned char needle = static_cast<unsigned char>(c);
  for(unsigned long i = 0; i < n; ++i) {
    if(bytes[i] == needle) {
      return const_cast<unsigned char *>(bytes + i);
    }
  }
  return nullptr;
}

extern "C" int cppgm_builtin_memcmp(const void * lhs, const void * rhs, unsigned long n)
{
  const unsigned char * left = static_cast<const unsigned char *>(lhs);
  const unsigned char * right = static_cast<const unsigned char *>(rhs);
  for(unsigned long i = 0; i < n; ++i) {
    if(left[i] != right[i]) {
      return left[i] < right[i] ? -1 : 1;
    }
  }
  return 0;
}

extern "C" void * cppgm_builtin_memcpy(void * dst, const void * src, unsigned long n)
{
  unsigned char * out = static_cast<unsigned char *>(dst);
  const unsigned char * in = static_cast<const unsigned char *>(src);
  for(unsigned long i = 0; i < n; ++i) {
    out[i] = in[i];
  }
  return dst;
}

extern "C" void * cppgm_builtin_memmove(void * dst, const void * src, unsigned long n)
{
  unsigned char * out = static_cast<unsigned char *>(dst);
  const unsigned char * in = static_cast<const unsigned char *>(src);
  if(out < in) {
    for(unsigned long i = 0; i < n; ++i) {
      out[i] = in[i];
    }
  } else if(out > in) {
    for(unsigned long i = n; i > 0; --i) {
      out[i - 1] = in[i - 1];
    }
  }
  return dst;
}

extern "C" long cppgm_builtin_expect(long value, long expected)
{
  (void) expected;
  return value;
}

extern "C" int cppgm_builtin_strcmp(const char * lhs, const char * rhs)
{
  return std::strcmp(lhs, rhs);
}

extern "C" char * cppgm_builtin_strchr(const char * s, int c)
{
  const char * found = std::strchr(s, c);
  return const_cast<char *>(found);
}

extern "C" unsigned long cppgm_builtin_strlen(const char * s)
{
  return std::strlen(s);
}

extern "C" void * cppgm_builtin_operator_new(unsigned long size)
{
  return host_allocate(size);
}

extern "C" void * cppgm_builtin_operator_new_aligned(unsigned long size,
                                                     unsigned long alignment)
{
  return host_allocate_aligned(size, alignment);
}

extern "C" void * cppgm_builtin_operator_new_array(unsigned long size)
{
  return host_allocate(size);
}

extern "C" void * cppgm_builtin_operator_new_array_aligned(unsigned long size,
                                                           unsigned long alignment)
{
  return host_allocate_aligned(size, alignment);
}

extern "C" void cppgm_builtin_operator_delete(void * ptr)
{
  std::free(ptr);
}

extern "C" void cppgm_builtin_operator_delete_sized(void * ptr, unsigned long size)
{
  (void) size;
  std::free(ptr);
}

extern "C" void cppgm_builtin_operator_delete_aligned(void * ptr, unsigned long alignment)
{
  (void) alignment;
  std::free(ptr);
}

extern "C" void cppgm_builtin_operator_delete_sized_aligned(void * ptr,
                                                            unsigned long size,
                                                            unsigned long alignment)
{
  (void) size;
  (void) alignment;
  std::free(ptr);
}

extern "C" void cppgm_builtin_operator_delete_array(void * ptr)
{
  std::free(ptr);
}

extern "C" void cppgm_builtin_operator_delete_array_sized(void * ptr, unsigned long size)
{
  (void) size;
  std::free(ptr);
}

extern "C" void cppgm_builtin_operator_delete_array_aligned(void * ptr,
                                                            unsigned long alignment)
{
  (void) alignment;
  std::free(ptr);
}

extern "C" void cppgm_builtin_operator_delete_array_sized_aligned(void * ptr,
                                                                  unsigned long size,
                                                                  unsigned long alignment)
{
  (void) size;
  (void) alignment;
  std::free(ptr);
}

extern "C" void cppgm_builtin_unreachable()
{
  std::abort();
}

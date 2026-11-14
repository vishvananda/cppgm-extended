#include "driver/wasm_environment.h"

#include "driver/host_environment.h"

namespace driver {

namespace {

// Fixed IEEE binary32/binary64/binary128 limits of this target profile.
const char kNumericPredefines[] = "#define __DBL_DECIMAL_DIG__ 17\n"
								  "#define __DBL_DENORM_MIN__ 4.9406564584124654e-324\n"
								  "#define __DBL_DIG__ 15\n"
								  "#define __DBL_EPSILON__ 2.2204460492503131e-16\n"
								  "#define __DBL_HAS_DENORM__ 1\n"
								  "#define __DBL_HAS_INFINITY__ 1\n"
								  "#define __DBL_HAS_QUIET_NAN__ 1\n"
								  "#define __DBL_MANT_DIG__ 53\n"
								  "#define __DBL_MAX_10_EXP__ 308\n"
								  "#define __DBL_MAX_EXP__ 1024\n"
								  "#define __DBL_MAX__ 1.7976931348623157e+308\n"
								  "#define __DBL_MIN_10_EXP__ (-307)\n"
								  "#define __DBL_MIN_EXP__ (-1021)\n"
								  "#define __DBL_MIN__ 2.2250738585072014e-308\n"
								  "#define __DBL_NORM_MAX__ 1.7976931348623157e+308\n"
								  "#define __DECIMAL_DIG__ __LDBL_DECIMAL_DIG__\n"
								  "#define __FLT_DECIMAL_DIG__ 9\n"
								  "#define __FLT_DENORM_MIN__ 1.40129846e-45F\n"
								  "#define __FLT_DIG__ 6\n"
								  "#define __FLT_EPSILON__ 1.19209290e-7F\n"
								  "#define __FLT_HAS_DENORM__ 1\n"
								  "#define __FLT_HAS_INFINITY__ 1\n"
								  "#define __FLT_HAS_QUIET_NAN__ 1\n"
								  "#define __FLT_MANT_DIG__ 24\n"
								  "#define __FLT_MAX_10_EXP__ 38\n"
								  "#define __FLT_MAX_EXP__ 128\n"
								  "#define __FLT_MAX__ 3.40282347e+38F\n"
								  "#define __FLT_MIN_10_EXP__ (-37)\n"
								  "#define __FLT_MIN_EXP__ (-125)\n"
								  "#define __FLT_MIN__ 1.17549435e-38F\n"
								  "#define __FLT_NORM_MAX__ 3.40282347e+38F\n"
								  "#define __FLT_RADIX__ 2\n"
								  "#define __INT_MAX__ 2147483647\n"
								  "#define __LDBL_DECIMAL_DIG__ 36\n"
								  "#define __LDBL_DENORM_MIN__ 6.47517511943802511092443895822764655e-4966L\n"
								  "#define __LDBL_DIG__ 33\n"
								  "#define __LDBL_EPSILON__ 1.92592994438723585305597794258492732e-34L\n"
								  "#define __LDBL_HAS_DENORM__ 1\n"
								  "#define __LDBL_HAS_INFINITY__ 1\n"
								  "#define __LDBL_HAS_QUIET_NAN__ 1\n"
								  "#define __LDBL_MANT_DIG__ 113\n"
								  "#define __LDBL_MAX_10_EXP__ 4932\n"
								  "#define __LDBL_MAX_EXP__ 16384\n"
								  "#define __LDBL_MAX__ 1.18973149535723176508575932662800702e+4932L\n"
								  "#define __LDBL_MIN_10_EXP__ (-4931)\n"
								  "#define __LDBL_MIN_EXP__ (-16381)\n"
								  "#define __LDBL_MIN__ 3.36210314311209350626267781732175260e-4932L\n"
								  "#define __LDBL_NORM_MAX__ 1.18973149535723176508575932662800702e+4932L\n"
								  "#define __LONG_LONG_MAX__ 9223372036854775807LL\n"
								  "#define __SCHAR_MAX__ 127\n"
								  "#define __SHRT_MAX__ 32767\n";

} // namespace

void add_wasm_headers(PreprocessorOptions& options, const std::string& sysroot, const std::string& resource_directory)
{
	if (!sysroot.empty())
	{
		options.system_include_paths.push_back(
			sysroot + "/include/wasm32-wasip1/" + (options.modes.exceptions ? "eh" : "noeh") + "/c++/v1");
		options.system_include_paths.push_back(sysroot + "/include/wasm32-wasip1/c++/v1");
		options.system_include_paths.push_back(sysroot + "/include/c++/v1");
	}
	if (!resource_directory.empty())
		options.system_include_paths.push_back(resource_directory + "/include");
	if (!sysroot.empty())
	{
		options.system_include_paths.push_back(sysroot + "/include/wasm32-wasip1");
		options.system_include_paths.push_back(sysroot + "/include");
	}
}

void configure_wasm_environment(PreprocessorOptions& options)
{
	// Host predefined macros, include directories and va_list declarations
	// cannot describe a cross target. Target headers are supplied explicitly.
	options.host_predefines.clear();
	options.hosted = true;
	options.builtin_declarations = "typedef char* __builtin_va_list;\n";
	options.builtin_declarations += builtin_template_declarations();
	options.popcnt = false;
	options.sse_level = 0;
	std::string text = "#undef __linux__\n#undef __linux\n#undef __gnu_linux__\n#undef __ELF__\n"
					   "#undef __APPLE__\n#undef __MACH__\n#undef __x86_64__\n#undef __SSE__\n#undef __SSE2__\n"
					   "#define __wasm__ 1\n#define __wasm32__ 1\n#define __ILP32__ 1\n"
					   "#define __CHAR_BIT__ 8\n#define __SIZEOF_POINTER__ 4\n#define __SIZEOF_LONG__ 4\n"
					   "#define __SIZEOF_INT__ 4\n#define __SIZEOF_SHORT__ 2\n#define __SIZEOF_LONG_LONG__ 8\n"
					   "#define __SIZEOF_FLOAT__ 4\n#define __SIZEOF_DOUBLE__ 8\n#define __SIZEOF_LONG_DOUBLE__ 16\n"
					   "#define __SIZEOF_SIZE_T__ 4\n#define __SIZEOF_PTRDIFF_T__ 4\n"
					   "#define __SIZE_TYPE__ long unsigned int\n#define __PTRDIFF_TYPE__ long int\n"
					   "#define __INTPTR_TYPE__ long int\n#define __UINTPTR_TYPE__ long unsigned int\n"
					   "#define __INT64_TYPE__ long long int\n#define __UINT64_TYPE__ long long unsigned int\n"
					   "#define __INT32_TYPE__ int\n#define __UINT32_TYPE__ unsigned int\n"
					   "#define __WCHAR_TYPE__ int\n#define __WCHAR_MAX__ 2147483647\n"
					   "#define __WCHAR_MIN__ (-2147483647 - 1)\n#define __WCHAR_WIDTH__ 32\n"
					   "#define __WINT_TYPE__ int\n#define __WINT_MAX__ 2147483647\n"
					   "#define __WINT_MIN__ (-2147483647 - 1)\n#define __WINT_WIDTH__ 32\n"
					   "#define __ATOMIC_RELAXED 0\n#define __ATOMIC_CONSUME 1\n#define __ATOMIC_ACQUIRE 2\n"
					   "#define __ATOMIC_RELEASE 3\n#define __ATOMIC_ACQ_REL 4\n#define __ATOMIC_SEQ_CST 5\n"
					   "#define __LONG_MAX__ 2147483647L\n#define __SIZE_MAX__ 4294967295UL\n"
					   "#define __INTPTR_MAX__ 2147483647L\n#define __UINTPTR_MAX__ 4294967295UL\n"
					   "#define __PTRDIFF_MAX__ 2147483647L\n"
					   "#define __ORDER_LITTLE_ENDIAN__ 1234\n#define __ORDER_BIG_ENDIAN__ 4321\n"
					   "#define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__\n";
	text += kNumericPredefines;
	if (options.target == target_model::Target::Wasi)
		text += "#define __wasi__ 1\n";
	options.command_line.insert(0, text);
}

} // namespace driver

#include "driver/wasm_environment.h"

namespace driver {

void configure_wasm_environment(PreprocessorOptions& options)
{
	// Host predefined macros, include directories and va_list declarations
	// cannot describe a cross target. Target headers are supplied explicitly.
	options.host_predefines.clear();
	options.builtin_declarations = "typedef char* __builtin_va_list;\n";
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
					   "#define __LONG_MAX__ 2147483647L\n#define __SIZE_MAX__ 4294967295UL\n"
					   "#define __PTRDIFF_MAX__ 2147483647L\n#define __LDBL_MANT_DIG__ 113\n"
					   "#define __LDBL_MAX_EXP__ 16384\n#define __LDBL_MIN_EXP__ (-16381)\n"
					   "#define __FLT_MANT_DIG__ 24\n#define __DBL_MANT_DIG__ 53\n"
					   "#define __ORDER_LITTLE_ENDIAN__ 1234\n#define __ORDER_BIG_ENDIAN__ 4321\n"
					   "#define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__\n";
	if (options.target == target_model::Target::Wasi)
		text += "#define __wasi__ 1\n";
	options.command_line.insert(0, text);
}

} // namespace driver

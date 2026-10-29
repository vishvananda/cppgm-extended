extern "C" int optional_function() noexcept {return 23;}
namespace hooks {long optional_variable=0x123456789L;bool& enabled() noexcept {static bool value=true;return value;}}

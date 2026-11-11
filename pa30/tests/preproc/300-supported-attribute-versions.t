#if __has_cpp_attribute(no_unique_address) == 201803L && __has_cpp_attribute(noreturn) == 200809L
standard_attribute_versions
#else
bad_standard_attribute_versions
#endif
#if __has_cpp_attribute(gnu::always_inline) && __has_cpp_attribute(__gnu__::__pure__) && __has_attribute(naked)
implemented_gnu_attributes
#else
bad_gnu_attributes
#endif
#if __has_cpp_attribute(other::always_inline) || __has_cpp_attribute(gnu::imaginary_attribute)
bad_unknown_attribute
#endif

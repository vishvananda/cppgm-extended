#if __has_attribute(deprecated) && __has_attribute(__deprecated__)
gnu_deprecated_available
#else
unavailable
#endif
#if __has_attribute(cppgm_unrecognized_deprecated)
unexpected
#else
unknown_attribute_still_absent
#endif

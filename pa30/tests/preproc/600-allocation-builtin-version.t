#if __has_builtin(__builtin_operator_new) >= 201802L && __has_builtin(__builtin_operator_delete) >= 201802L
replaceable_allocation_overloads
#else
bad_allocation_builtin_version
#endif

#if __SIZEOF_POINTER__ != 4 || __SIZEOF_LONG__ != 4
#error incorrect target include environment
#endif
inline int supplied_target_value() { return sizeof(void*) + sizeof(long); }

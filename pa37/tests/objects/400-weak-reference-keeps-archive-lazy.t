extern "C" int optional_feature(int) __attribute__((weak));
extern "C" int observe() {
 return optional_feature ? optional_feature(1) : 17;
}

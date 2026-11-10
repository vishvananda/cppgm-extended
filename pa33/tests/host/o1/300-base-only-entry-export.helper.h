struct ExportBox { int value; };
struct ExportValue {
  int value;
  __attribute__((noinline)) ExportValue(const ExportBox& b) : value(b.value) {}
};
struct ExportDerived : ExportValue {
  explicit ExportDerived(const ExportBox& b) : ExportValue(b) {}
};
extern "C" int base_only_value(int);
extern "C" int complete_object_value(int);
extern "C" int host_check_base_only_exports();

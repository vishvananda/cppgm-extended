namespace member_template_pointer_target {
struct C {
  template<class T> void set(T) {}
};
}

int main()
{
  typedef void (member_template_pointer_target::C::*setter_type)(int);
  setter_type setter = &member_template_pointer_target::C::set;
  (void)setter;
  return 0;
}

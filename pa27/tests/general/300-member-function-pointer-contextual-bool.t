struct member_pointer_contextual_bool
{
  int f()
  {
    return 7;
  }
};

int main()
{
  int (member_pointer_contextual_bool::*member)() =
    &member_pointer_contextual_bool::f;
  if(!member) {
    return 1;
  }

  member = 0;
  if(member) {
    return 2;
  }
  return 0;
}

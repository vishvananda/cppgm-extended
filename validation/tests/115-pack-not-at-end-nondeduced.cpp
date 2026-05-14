template<typename T1, typename... Types>
int bad_pack(Types..., T1)
{
  return 0;
}

int main()
{
  return bad_pack(1, 2, 3);
}

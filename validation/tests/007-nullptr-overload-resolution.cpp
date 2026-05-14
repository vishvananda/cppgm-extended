// VALIDATION: run-pass
// N3485 focus: 2.14.7 [lex.nullptr], 4.10 [conv.ptr], 13.3 [over.match]

int which(int)
{
  return 1;
}

int which(int *)
{
  return 2;
}

int main()
{
  return which(nullptr) == 2 ? 0 : 1;
}

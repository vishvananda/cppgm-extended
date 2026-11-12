extern "C" int group_read();
extern "C" void group_write(int);
extern "C" int inspect() { return group_read(); }
extern "C" int observe(int value)
{
    group_write(value);
    return group_read() == value ? 17 : 1;
}

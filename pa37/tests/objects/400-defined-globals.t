extern "C" int numeric_globals(int);
extern "C" int null_global_value();
extern "C" int observe(int value)
{
    return null_global_value() == 1 ? numeric_globals(value) : -999;
}

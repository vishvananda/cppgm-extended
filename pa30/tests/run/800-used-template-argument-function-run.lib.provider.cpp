template<class T> int retained();
extern "C" int call_retained(){return retained<int>()+retained<long>();}

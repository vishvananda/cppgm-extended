// expect exit 0
// Adopted CWG727: explicit specialization shares the primary template definition scope.
class Host { int data; template<class T> struct Item { static int get(Host& h){return h.data;} }; template<> struct Item<int> { static int get(Host& h){return h.data+1;} }; friend int read(Host&); public: Host():data(6){} }; int read(Host& h){return Host::Item<int>::get(h);} int main(){Host h;return read(h)!=7;}

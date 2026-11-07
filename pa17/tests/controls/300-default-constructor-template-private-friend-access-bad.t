// expect reject
// N3485 11,14.5.2: constructor template specializations retain primary access.
struct D { private: template<class T=int> D(){} friend struct F; }; struct F { D d; F(){} }; struct U { D d; U(){} };

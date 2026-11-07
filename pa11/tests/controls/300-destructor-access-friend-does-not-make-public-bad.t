// expect reject
// N3485 11: access belongs to the selected declaration, independent of earlier permitted uses.
struct D { D(){} private: ~D(){} friend struct F; }; struct F { D d; F():d(){} ~F(){} }; struct U { D d; U():d(){} ~U(){} };

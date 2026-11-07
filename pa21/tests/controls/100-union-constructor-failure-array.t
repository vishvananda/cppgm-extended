// expect exit 0
// CWG1866 adopted DR: initialized variants are destroyed on construction failure, excluded on destruction failure.
int made,dead; struct D { D(){++made;} ~D(){++dead;} }; union U { D d[2]; int n; U():d(){throw 7;} ~U(){} }; int main(){try{U u;}catch(int){return made==2 && dead==2?0:1;}return 2;}

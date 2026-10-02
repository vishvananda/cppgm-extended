int f(){return 3;}int main(){const auto& function=f;return function()!=3;}

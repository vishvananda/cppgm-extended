set -eu
nm "__OBJ1__" | c++filt | rg -q 'Box<int>::probe<false>\(\) const'
nm "__OBJ1__" | c++filt | rg -q 'Box<int>::probe<true>\(\) const'
echo bool_member_template_specializations 2


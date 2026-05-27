set -eu
nm "__OBJ1__" | c++filt | rg -Fq 'function<int (unsigned long)>::operator()(unsigned long) const'
echo host_partial_specialization_ooc_member_definition 1

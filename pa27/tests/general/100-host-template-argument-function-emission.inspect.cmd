set -eu
if nm -C --defined-only "__OBJ1__" | grep -q 'check<Concept>'; then
 exit 1
fi
echo argument_only_function_not_emitted 1

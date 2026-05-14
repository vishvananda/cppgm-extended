set -eu
nm "__OBJ2__" | grep -Eq '(_)?ZNK3BoxntEv$'
if nm "__OBJ2__" | grep -q '11operator_21EU'; then
  echo unexpected_weak_operator_fallback
  exit 1
fi
echo member_operator_bang_ok 1

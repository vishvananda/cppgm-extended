set -e
count=$(
  nm -an __OBJ2__ \
    | c++filt \
    | grep 'basic_ostream<char, .*>::basic_ostream.*basic_streambuf<char,' \
    | wc -l \
    | tr -d '[:space:]'
)
if [ "$count" -ne 1 ]; then
  echo "hosted_ostream_complete_ctor_unaliased 0"
  exit 1
fi
echo "hosted_ostream_complete_ctor_unaliased 1"

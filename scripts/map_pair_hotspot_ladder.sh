#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BINARY="${BINARY:-$ROOT/dev/cppgm++}"
HOST_CXX="${CPPGM_HOST_CXX:-/usr/local/opt/llvm/bin/clang++}"
FILTER="${HOTSPOT_FILTER:-pair<}"
DUMP_LIMIT="${HOTSPOT_DUMP_LIMIT:-256}"
WORKDIR="${HOTSPOT_WORKDIR:-/tmp/cppgm-map-pair-ladder}"

mkdir -p "$WORKDIR"

emit_case() {
  local name="$1"
  local body="$2"
  local src="$WORKDIR/$name.cpp"
  local obj="$WORKDIR/$name.o"
  local stderr_file="$WORKDIR/$name.stderr"
  local status="ok"

  printf '%s\n' "$body" > "$src"

  if ! env \
      CPPGM_SEMANTIC_HOTSPOT=1 \
      CPPGM_SEMANTIC_HOTSPOT_DUMP_QUERY="$FILTER" \
      CPPGM_SEMANTIC_HOTSPOT_DUMP_LIMIT="$DUMP_LIMIT" \
      CPPGM_HOST_CXX="$HOST_CXX" \
      "$BINARY" -c -I"$ROOT/dev/src" "$src" -o "$obj" \
      >/dev/null 2>"$stderr_file"; then
    status="fail"
  fi

  local summary
  summary="$(rg -m1 '^SEMANTIC_HOTSPOT summary' "$stderr_file" || true)"
  local dump_header
  dump_header="$(rg -m1 '^SEMANTIC_HOTSPOT query_dump ' "$stderr_file" || true)"
  local pair_stats
  pair_stats="$(awk '
    /^SEMANTIC_HOTSPOT query_dump / {
      in_dump=1
      for(i = 1; i <= NF; ++i) {
        if($i ~ /^matches=/) {
          value = $i
          sub(/^matches=/, "", value)
          matches = value + 0
        }
      }
      next
    }
    /^SEMANTIC_HOTSPOT / && in_dump {exit}
    in_dump && /^  count=/ {
      ++rows
      for(i = 1; i <= NF; ++i) {
        if($i ~ /^count=/) {
          value = $i
          sub(/^count=/, "", value)
          total += value + 0
        }
      }
    }
    END {
      printf("matches=%d dumped=%d total=%d", matches, rows, total)
    }
  ' "$stderr_file")"

  printf '== %s ==\n' "$name"
  printf 'status: %s\n' "$status"
  if [[ -n "$summary" ]]; then
    printf '%s\n' "$summary"
  else
    printf 'SEMANTIC_HOTSPOT summary <missing>\n'
  fi
  printf 'pair_queries: %s\n' "$pair_stats"
  awk '
    /^SEMANTIC_HOTSPOT query_dump / {in_dump=1; next}
    /^SEMANTIC_HOTSPOT / && in_dump {exit}
    in_dump && /^  count=/ {
      print
      ++printed
      if(printed == 8) {
        exit
      }
    }
  ' "$stderr_file"
  printf 'stderr: %s\n' "$stderr_file"
  printf '\n'
}

emit_case "map_header" '#include <map>
int main() { return 0; }'

emit_case "map_alias" '#include <map>
using Map = std::map<int, int>;
int main() { return 0; }'

emit_case "map_value_type" '#include <map>
using Value = std::map<int, int>::value_type;
int main() { return 0; }'

emit_case "map_decl" '#include <map>
int main() {
  std::map<int, int> m;
  return static_cast<int>(m.size());
}'

emit_case "map_find" '#include <map>
int main() {
  std::map<int, int> m;
  return m.find(0) == m.end();
}'

emit_case "pair_decl" '#include <map>
int main() {
  std::pair<int, int> p(1, 2);
  return p.first;
}'

emit_case "make_pair" '#include <map>
int main() {
  std::pair<int, int> p = std::make_pair(1, 2);
  return p.first;
}'

emit_case "pair_convert_lvalue" '#include <map>
int main() {
  std::pair<int, int> p(1, 2);
  std::pair<const int, int> q(p);
  return q.first;
}'

emit_case "pair_is_constructible_lvalue" '#include <map>
#include <type_traits>
int main() {
  return std::is_constructible<
      std::pair<const int, int>,
      std::pair<int, int> &>::value ? 0 : 1;
}'

emit_case "map_subscript" '#include <map>
int main() {
  std::map<int, int> m;
  m[1] = 2;
  return m[1];
}'

emit_case "map_insert_lvalue_pair" '#include <map>
int main() {
  std::map<int, int> m;
  std::pair<int, int> p(1, 2);
  m.insert(p);
  return static_cast<int>(m.size());
}'

emit_case "map_insert_value_type" '#include <map>
int main() {
  std::map<int, int> m;
  std::map<int, int>::value_type v(1, 2);
  m.insert(v);
  return static_cast<int>(m.size());
}'

emit_case "map_insert" '#include <map>
int main() {
  std::map<int, int> m;
  m.insert(std::make_pair(1, 2));
  return static_cast<int>(m.size());
}'

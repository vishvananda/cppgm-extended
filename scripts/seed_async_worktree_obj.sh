#!/usr/bin/env bash
set -euo pipefail

usage() {
  echo "usage: $0 <source-worktree> [dest-worktree]" >&2
  exit 2
}

if [[ $# -lt 1 || $# -gt 2 ]]; then
  usage
fi

source_root=$(cd "$1" && pwd)
dest_root=$(cd "${2:-.}" && pwd)

if [[ "$source_root" == "$dest_root" ]]; then
  echo "source and destination worktrees must differ" >&2
  exit 1
fi

source_head=$(
  cd "$source_root"
  git rev-parse HEAD
)
dest_head=$(
  cd "$dest_root"
  git rev-parse HEAD
)

if [[ "$source_head" != "$dest_head" ]]; then
  echo "refusing to seed obj across different HEADs" >&2
  echo "source: $source_head ($source_root)" >&2
  echo "dest:   $dest_head ($dest_root)" >&2
  exit 1
fi

if [[ ! -d "$source_root/obj" ]]; then
  echo "source obj directory not found: $source_root/obj" >&2
  exit 1
fi

mkdir -p "$dest_root/obj"

if command -v rsync >/dev/null 2>&1; then
  rsync -a --delete "$source_root/obj/" "$dest_root/obj/"
else
  rm -rf "$dest_root/obj"
  mkdir -p "$dest_root/obj"
  cp -R "$source_root/obj/." "$dest_root/obj/"
fi

# Fresh worktree checkouts often have source-file mtimes newer than the copied objects.
# Refresh the copied tree so narrow rebuilds can actually reuse the seeded outputs.
find "$dest_root/obj" -type f -exec touch {} +

echo "seeded $dest_root/obj from $source_root/obj at HEAD $source_head"

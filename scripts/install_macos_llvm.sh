#!/usr/bin/env bash
# Install the pinned Intel Homebrew toolchain used by the macOS CI lane.
set -euo pipefail

major=${1:-21}
case "$major" in
  18|21) ;;
  *) echo "usage: $0 [18|21]" >&2; exit 2 ;;
esac
if [[ $(uname -s) != Darwin || $(uname -m) != x86_64 ]]; then
  echo "This installer requires Intel macOS." >&2
  exit 2
fi

export HOMEBREW_NO_AUTO_UPDATE=1
if ! brew tap-info cppgm/ci >/dev/null 2>&1; then
  brew tap-new cppgm/ci
fi
formula="$(brew --repository cppgm/ci)/Formula/llvm@$major.rb"
curl --fail --location --retry 3 \
  "https://raw.githubusercontent.com/Homebrew/homebrew-core/c722500ede2e1d33523c27ef11338fec48adfdf9/Formula/l/llvm@$major.rb" \
  --output "$formula"
python3 - "$formula" <<'PY'
from pathlib import Path
import sys
path = Path(sys.argv[1])
text = path.read_text()
text = text.replace('  bottle do\n',
                    '  bottle do\n    root_url "https://ghcr.io/v2/homebrew/core"\n', 1)
path.write_text(text)
PY
brew install --force-bottle "cppgm/ci/llvm@$major"
printf '\nHost compiler: %s/bin/clang++\n' "$(brew --prefix "cppgm/ci/llvm@$major")"

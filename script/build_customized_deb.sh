#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
echo "build_customized_deb.sh is deprecated; use build_throne_sni_deb.sh instead." >&2
exec "$SCRIPT_DIR/build_throne_sni_deb.sh" "$@"

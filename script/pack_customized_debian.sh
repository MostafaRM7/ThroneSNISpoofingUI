#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
echo "pack_customized_debian.sh is deprecated; use pack_throne_sni_debian.sh instead." >&2
exec "$SCRIPT_DIR/pack_throne_sni_debian.sh" "$@"

#!/bin/bash
set -e

VERSION="${1:-0.1.0}"
ARCH="amd64"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$SCRIPT_DIR/.."
DEPLOYMENT_DIR="$ROOT_DIR/deployment"
SNI_DIR="$ROOT_DIR/resources/snispoof"

if [[ ! -f "$SNI_DIR/snispoof" ]]; then
    echo "Missing SNI binary: $SNI_DIR/snispoof" >&2
    exit 1
fi
if [[ ! -f "$SNI_DIR/snispoof-config.json" ]]; then
    echo "Missing SNI config template: $SNI_DIR/snispoof-config.json" >&2
    exit 1
fi
chmod 0755 "$SNI_DIR/snispoof"

if [[ ! -d "$DEPLOYMENT_DIR/linux-amd64" ]]; then
    echo "Missing deployment/linux-amd64. Build and deploy Throne first, then rerun this script." >&2
    echo "CI does this with script/deploy_linux64.sh before packaging." >&2
    exit 1
fi

cd "$DEPLOYMENT_DIR"
bash "$SCRIPT_DIR/pack_throne_sni_debian.sh" "$VERSION" "$ARCH"

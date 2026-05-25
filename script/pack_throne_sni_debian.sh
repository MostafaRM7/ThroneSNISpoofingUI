#!/bin/bash
set -e

VERSION="$1"
ARCH="$2"
SYSTEMQT_SUFFIX=""
if [[ ${3:-} == "systemqt" ]]; then
    SYSTEMQT_SUFFIX="-system-qt"
fi

if [[ -z "$VERSION" || -z "$ARCH" ]]; then
    echo "Usage: $0 <version> <arch> [systemqt]" >&2
    exit 1
fi

if [[ "$ARCH" != "amd64" ]]; then
    echo "throne-sni currently bundles only the Linux amd64 SNI-Spoofing-Go binary" >&2
    exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$SCRIPT_DIR/.."
RES_LINUX="$ROOT_DIR/res/linux"
SNI_DIR="$ROOT_DIR/resources/snispoof"
SNI_BIN="$SNI_DIR/snispoof"
SNI_CONFIG="$SNI_DIR/snispoof-config.json"
PKG_DIR="throne-sni"
APP_DIR="$PKG_DIR/opt/throne-sni"

if [[ ! -f "$SNI_BIN" ]]; then
    echo "Missing SNI binary: $SNI_BIN" >&2
    exit 1
fi
if [[ ! -f "$SNI_CONFIG" ]]; then
    echo "Missing SNI config template: $SNI_CONFIG" >&2
    exit 1
fi

rm -rf "$PKG_DIR" throne-sni.deb
mkdir -p "$PKG_DIR/opt"
cp -r "linux-${ARCH}${SYSTEMQT_SUFFIX}" "$PKG_DIR/opt"
mv "$PKG_DIR/opt/linux-${ARCH}${SYSTEMQT_SUFFIX}" "$APP_DIR"
rm -f "$APP_DIR/Throne.debug"

install -D -m 0755 "$SNI_BIN" "$APP_DIR/snispoof/snispoof"
install -D -m 0644 "$SNI_CONFIG" "$APP_DIR/snispoof/snispoof-config.json"

install -d "$PKG_DIR/usr/bin"
cat >"$PKG_DIR/usr/bin/throne-sni" <<'EOF'
#!/bin/sh
exec env PATH="/opt/throne-sni:$PATH" /opt/throne-sni/Throne -appdata "$HOME/.config/throne-sni" "$@"
EOF
chmod 0755 "$PKG_DIR/usr/bin/throne-sni"

install -d "$PKG_DIR/usr/share/applications"
cat >"$PKG_DIR/usr/share/applications/throne-sni.desktop" <<'EOF'
[Desktop Entry]
Type=Application
Name=Throne SNI
GenericName=Proxy Configuration Manager
Comment=Throne GUI proxy manager with bundled SNI-Spoofing-Go integration
Exec=throne-sni
Icon=throne-sni
Terminal=false
Categories=Network;
Keywords=proxy;vpn;singbox;sing-box;tun;sni;
StartupNotify=true
StartupWMClass=Throne
EOF

install -d "$PKG_DIR/usr/share/metainfo"
sed -e "s|@@VERSION@@|${VERSION}|g" \
    -e "s|@@DATE@@|$(date -u +%Y-%m-%d)|g" \
    -e "s|<id>org.throneproj.Throne</id>|<id>io.github.flashboy.ThroneSNI</id>|g" \
    -e "s|<name>Throne</name>|<name>Throne SNI</name>|g" \
    -e "s|<summary>Qt based cross-platform GUI proxy configuration manager</summary>|<summary>Throne proxy GUI with bundled SNI-Spoofing-Go integration</summary>|g" \
    -e "s|org.throneproj.Throne.desktop|throne-sni.desktop|g" \
    -e "s|<binary>Throne</binary>|<binary>throne-sni</binary>|g" \
    "$RES_LINUX/org.throneproj.Throne.metainfo.xml" \
    > "$PKG_DIR/usr/share/metainfo/io.github.flashboy.ThroneSNI.metainfo.xml"

for size in 16x16 32x32 48x48 64x64 128x128 256x256 512x512; do
    install -D -m 0644 \
        "$RES_LINUX/icons/hicolor/${size}/apps/org.throneproj.Throne.png" \
        "$PKG_DIR/usr/share/icons/hicolor/${size}/apps/throne-sni.png"
done

mkdir -p "$PKG_DIR/DEBIAN"
cat >"$PKG_DIR/DEBIAN/control" <<EOF
Package: throne-sni
Version: $VERSION
Architecture: $ARCH
Maintainer: Mahdi mahdi.pgitmail@gmail.com
Depends: desktop-file-utils, libcap2-bin$([[ ${3:-} == "systemqt" ]] && echo ", libqt6core6, libqt6gui6, libqt6network6, libqt6widgets6, qt6-qpa-plugins, qt6-wayland, qt6-gtk-platformtheme, qt6-xdgdesktopportal-platformtheme, libxcb-cursor0, fonts-noto-color-emoji")
Conflicts: customized-throne
Replaces: customized-throne
Description: Throne SNI proxy manager with bundled SNI-Spoofing-Go integration
EOF

cat >"$PKG_DIR/DEBIAN/postinst" <<'EOF'
#!/bin/sh
set -e
chmod 0755 /opt/throne-sni/snispoof/snispoof || true
setcap cap_net_raw+ep /opt/throne-sni/snispoof/snispoof || true
update-desktop-database -q || true
EOF
chmod 0755 "$PKG_DIR/DEBIAN/postinst"

dpkg-deb --build "$PKG_DIR" throne-sni.deb

# Throne SNI

Throne SNI is a Linux-focused build of [Throne](https://github.com/throneproj/Throne) with bundled [SNI-Spoofing-Go](https://github.com/aleskxyz/SNI-Spoofing-Go) integration.

It keeps Throne as the base Qt desktop proxy GUI, powered by sing-box and Xray/V2Ray support, and adds a packaged local SNI spoofing helper so users can install one `.deb` and run everything offline without downloading extra binaries.

## What this build adds

Compared with upstream Throne, this build adds:

- A bundled Linux amd64 SNI-Spoofing-Go binary.
- A bundled default SNI config template.
- A main-window **SNI Spoof** toggle and running/stopped status indicator.
- A profile context-menu action: **Connect via SNI Spoof**.
- Runtime SNI config generation.
- Debian packaging under the package name `throne-sni`.
- A separate launcher command: `throne-sni`.
- Side-by-side install paths so upstream Throne can remain installed.

## How the SNI integration works

SNI-Spoofing-Go runs as a local helper process. By default it listens on:

```text
127.0.0.1:40443
```

When you choose **Connect via SNI Spoof** on a profile, Throne SNI:

1. Reads the selected profile's original host and port.
2. Writes a runtime SNI-Spoofing-Go config.
3. Starts the bundled SNI helper.
4. Temporarily starts the selected profile through the local SNI listener.
5. Does **not** permanently modify the saved profile.
6. Stops the helper when the SNI-backed profile stops or the app exits.

The SNI helper needs raw socket permission on Linux. The Debian package depends on `libcap2-bin` and runs this during install:

```sh
setcap cap_net_raw+ep /opt/throne-sni/snispoof/snispoof || true
```

That allows the bundled helper to run without launching the whole GUI with `sudo`.

## Installation

Install the local release package:

```bash
sudo dpkg -i throne-sni-v0.1.0-debian-amd64.deb
```

If `dpkg` reports missing dependencies, repair them with:

```bash
sudo apt -f install
```

Launch the app:

```bash
throne-sni
```

The package installs side-by-side with upstream Throne.

## Installed paths

The Debian package uses these paths:

```text
/opt/throne-sni/Throne
/opt/throne-sni/ThroneCore
/opt/throne-sni/snispoof/snispoof
/opt/throne-sni/snispoof/snispoof-config.json
/usr/bin/throne-sni
/usr/share/applications/throne-sni.desktop
/usr/share/metainfo/io.github.flashboy.ThroneSNI.metainfo.xml
$HOME/.config/throne-sni
```

The GUI executable remains named `Throne` internally to minimize divergence from upstream Throne. The package, launcher, desktop entry, and release artifacts are named `throne-sni`.

## Using SNI Spoof

### Configure SNI Spoof

Open:

```text
Settings → Basic Settings → Core → SNI Spoof
```

Available settings include:

- Enable SNI Spoof integration.
- Binary path override.
- Listen host and port.
- Default connect host/IP and port.
- Fake SNI.
- CLI arguments.
- Config preview.

### Start the helper directly

Use the main-window **SNI Spoof** toggle. The status label shows whether the helper is running.

### Connect a profile through SNI Spoof

Right-click one profile and choose:

```text
Connect via SNI Spoof
```

The action requires a profile with a valid server address and port.

## Default local proxy

The default mixed local proxy bind for Throne SNI is:

```text
0.0.0.0:2080
```

This is the mixed SOCKS+HTTP inbound inherited from Throne/sing-box.

## Supported protocols

Throne SNI preserves upstream Throne protocol support, including:

- SOCKS
- HTTP(S)
- Shadowsocks
- Trojan
- VMess
- VLESS
- TUIC
- Hysteria
- Hysteria2
- AnyTLS
- NaïveProxy
- Juicity
- TrustTunnel
- ShadowTLS
- WireGuard
- SSH
- Custom outbound
- Custom config
- Chained outbounds
- Extra core

## Subscription formats

Throne SNI inherits Throne's subscription support, including share links, sing-box JSON representations, v2rayN links, and limited Shadowsocks/Clash support.

## Build from source

### Required tools

Install the typical Ubuntu build dependencies:

```bash
sudo apt update
sudo apt install -y \
  cmake \
  g++ \
  ninja-build \
  qt6-base-dev \
  qt6-tools-dev \
  qt6-tools-dev-tools \
  qt6-l10n-tools \
  libqt6svg6-dev \
  libglx-dev \
  libgl1-mesa-dev \
  golang-go \
  protobuf-compiler \
  git \
  curl \
  wget \
  lld \
  libcap2-bin \
  desktop-file-utils
```

Install Go protobuf plugins:

```bash
export PATH="/usr/local/go/bin:$HOME/go/bin:$PATH"
go install github.com/golang/protobuf/protoc-gen-go@latest
go install google.golang.org/grpc/cmd/protoc-gen-go-grpc@latest
```

### Build GUI

```bash
cmake -S . -B build -GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo
curl -fLso build/srslist.h \
  "https://raw.githubusercontent.com/throneproj/routeprofiles/rule-set/srslist.h"
cmake --build build
```

If GitHub is only reachable through a SOCKS5 proxy, use:

```bash
curl --socks5-hostname 127.0.0.1:2080 -fLso build/srslist.h \
  "https://raw.githubusercontent.com/throneproj/routeprofiles/rule-set/srslist.h"
```

### Build core and package

The CI-style Go core build also needs the Cronet/NaïveProxy toolchain environment used by upstream Throne. Once `deployment/linux-amd64` contains `Throne`, `ThroneCore`, `Throne.png`, and `updater`, build the Debian package with:

```bash
script/build_throne_sni_deb.sh 0.1.0
```

The output package is:

```text
deployment/throne-sni.deb
```

The release artifact is renamed to:

```text
deployment/throne-sni-v0.1.0-debian-amd64.deb
```

## Release artifacts

For `v0.1.0`, the local release includes:

```text
throne-sni-v0.1.0-debian-amd64.deb
throne-sni-v0.1.0-debian-amd64.deb.sha256
RELEASE_NOTES-v0.1.0.md
```

## Linux notes

### TUN and privileged features

Creating and managing a system TUN interface requires elevated privileges. Throne SNI inherits Throne's Linux privilege behavior for TUN/DNS features.

### Raw sockets for SNI-Spoofing-Go

The bundled SNI helper needs `CAP_NET_RAW`. The package applies it automatically with `setcap`. To verify after install:

```bash
getcap /opt/throne-sni/snispoof/snispoof
```

Expected output:

```text
/opt/throne-sni/snispoof/snispoof cap_net_raw=ep
```

### System proxy recovery

If the app is force-quit while **System Proxy** is enabled, it may not reset the system proxy. Reopen the app, enable **System Proxy**, then disable it to reset system settings.

## Migration from test builds

Earlier local test packages were named `customized-throne` and used:

```text
/opt/customized-throne
/usr/bin/customized-throne
$HOME/.config/customized-throne
```

The release package is now named `throne-sni` and uses separate paths. If you want to keep old settings, copy them manually:

```bash
cp -a "$HOME/.config/customized-throne" "$HOME/.config/throne-sni"
```

## Credits

Throne SNI combines and builds on these projects:

- [Throne](https://github.com/throneproj/Throne)
- [SNI-Spoofing-Go](https://github.com/aleskxyz/SNI-Spoofing-Go)
- [SagerNet/sing-box](https://github.com/SagerNet/sing-box)
- [XTLS/Xray-core](https://github.com/xtls/xray-core)
- [Qt](https://www.qt.io/)
- [Qv2ray](https://github.com/Qv2ray/Qv2ray)
- [simple-protobuf](https://github.com/tonda-kriz/simple-protobuf)
- [fkYAML](https://github.com/fktn-k/fkYAML)
- [quirc](https://github.com/dlbeer/quirc)
- [QHotkey](https://github.com/Skycoder42/QHotkey)
- [srombauts/sqlitecpp](https://github.com/srombauts/sqlitecpp)

Route profile downloads come from [throneproj/routeprofiles](https://github.com/throneproj/routeprofiles).

## License

The Throne codebase is licensed under GPL-3.0-or-later. Bundled third-party components keep their own upstream licenses.

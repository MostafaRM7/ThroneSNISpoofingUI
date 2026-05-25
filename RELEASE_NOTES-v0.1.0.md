# Throne SNI v0.1.0

Initial local Debian release of Throne SNI.

## Highlights

- Based on the Throne Qt proxy GUI.
- Bundles SNI-Spoofing-Go for Linux amd64.
- Adds a main-window SNI Spoof toggle and running/stopped status.
- Adds the profile action `Connect via SNI Spoof`.
- Installs as the Debian package `throne-sni`.
- Provides the launcher command `throne-sni`.
- Keeps the upstream GUI executable name `Throne` internally to reduce divergence.

## Package

- Package: `throne-sni`
- Version: `0.1.0`
- Architecture: `amd64`
- Install path: `/opt/throne-sni`
- Launcher: `/usr/bin/throne-sni`
- User config path: `$HOME/.config/throne-sni`
- Bundled SNI binary: `/opt/throne-sni/snispoof/snispoof`
- Bundled SNI config template: `/opt/throne-sni/snispoof/snispoof-config.json`

## Network defaults

- Throne mixed local proxy default: `0.0.0.0:2080`
- SNI-Spoofing-Go default listener: `127.0.0.1:40443`

## Linux permissions

The package depends on `libcap2-bin` and applies:

```sh
setcap cap_net_raw+ep /opt/throne-sni/snispoof/snispoof || true
```

This lets the bundled SNI helper open raw sockets without running the whole app with `sudo`.

## Checksums

See `throne-sni-v0.1.0-debian-amd64.deb.sha256`.

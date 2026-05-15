# Nyx

Cross-platform LoL chat proxy — appear offline / invisible / mobile without touching the game client.

Inspired by [Deceive](https://github.com/molenzwiebel/Deceive). Same technique: local TLS MITM on the XMPP chat connection. No memory patching, no DLL injection, no game-traffic interference.

## Status

Pre-alpha scaffold. Cert generation, OS trust store install, and the actual XMPP rewriter are stubs (see `TODO(phase-*)` markers).

## Stack

- **C++17**, CMake
- **Qt 6.5+** (Core / Gui / Qml / Quick / QuickControls2 / Network) — UI via QML
- **OpenSSL** — TLS server + client + X509 generation
- Per-OS native trust store APIs (Security.framework / wincrypt / Wine registry)

## Layout

```
src/
  main.cpp                 entry, wires QML <-> AppController
  core/                    proxy, xmpp rewriter, cert gen, system.yaml patcher, riot paths, config
  platform/                trust_mac.mm / trust_win.cpp / trust_linux.cpp
  ui/                      AppController + Roster/Presence models
qml/                       Main + Theme + screens + components
resources/                 icons
installers/                MSI / DMG / AppImage (TBD)
```

## Build

```
brew install qt openssl@3        # macOS
# or: sudo apt install qt6-base-dev qt6-declarative-dev libssl-dev
cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix qt);$(brew --prefix openssl@3)"
cmake --build build -j
./build/nyx.app/Contents/MacOS/nyx   # or build/nyx on Linux / build\nyx.exe on Windows
```

## Phases

1. `cert` — X509 CA + leaf, OS trust install.
2. `proxy` — TLS MITM on 127.0.0.1:5223, upstream to real Riot chat host.
3. `patcher` — `system.yaml` chat_host rewrite, backup/restore.
4. `xmpp` — streaming stanza rewriter (Online/Away/Mobile/Invisible/Offline).
5. `ui` — QML polish, tray icon, autostart.
6. `installers` — MSI / DMG / AppImage.

## License

TBD.

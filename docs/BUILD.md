# Build

## Dependencies

| OS      | Qt 6.5+                   | OpenSSL          | Compiler        |
|---------|---------------------------|------------------|-----------------|
| macOS   | `brew install qt`         | `brew install openssl@3` | Xcode CLT |
| Linux   | `qt6-base-dev qt6-declarative-dev qt6-quickcontrols2-dev` | `libssl-dev` | gcc 11+ / clang 14+ |
| Windows | Qt online installer (MSVC 2022 64-bit) | Qt's bundled or vcpkg `openssl` | VS 2022 |

## Configure + build

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="<qt-prefix>;<openssl-prefix>"
cmake --build build -j
```

## Run (macOS)

```
./build/nyx.app/Contents/MacOS/nyx
```

The Riot Client must NOT be running when patching `system.yaml`. Cert install requires admin/sudo prompt on first launch.

## Test on Wine (Linux LoL)

The patcher writes to the Wine prefix's copy of `system.yaml`. Set `WINEPREFIX` before launching Nyx.

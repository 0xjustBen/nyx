# Contributing

## Build

See [docs/WINDOWS.md](docs/WINDOWS.md) for Windows. macOS:

```
brew install qt openssl@3 cmake ninja
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH="$(brew --prefix qt);$(brew --prefix openssl@3)"
cmake --build build -j
./build/nyx.app/Contents/MacOS/nyx
```

Linux:

```
sudo apt install qt6-base-dev qt6-declarative-dev qt6-quickcontrols2-dev libssl-dev cmake ninja-build
cmake -S . -B build -G Ninja && cmake --build build -j
./build/nyx
```

## Tests

```
cmake -S . -B build -DNYX_BUILD_TESTS=ON && cmake --build build
ctest --test-dir build --output-on-failure
```

## PR checklist

- [ ] `cmake --build` clean on macOS + Windows CI
- [ ] `ctest` green
- [ ] No new clang warnings
- [ ] Updated docs if user-facing
- [ ] Smoke-tested against a real Riot Client login if proxy-path changed

## Code layout

```
src/core/      — proxy, cert, xmpp, configproxy, launcher, roster_store, config
src/ui/        — Qt models + AppController bridge
src/platform/  — per-OS trust store install (trust_{mac,win,linux})
qml/           — Main, screens/, components/, Theme singleton
tests/         — *_smoke.cpp unit + integration
docs/          — design spec, build, smoke-test docs
.github/       — workflows
```

## Commit style

Imperative subject under 72 chars. Body wraps at 72. Reference component
(`ui:`, `core:`, `ci:`, `build:`).

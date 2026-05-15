# Nyx · Windows

Build, install, run, debug on Windows 10/11 x64.

## Two ways to get nyx.exe

### A. GitHub Actions (no local toolchain)

1. Push repo to GitHub.
2. Workflow `.github/workflows/windows.yml` triggers on `push` to `main`/`master` or `workflow_dispatch`.
3. Download artifact `nyx-windows-x64.zip` from the run page.
4. Unzip anywhere. Run `nyx.exe`.

### B. Local build

Requirements:
- Visual Studio 2022 (Community OK) — "Desktop development with C++"
- Qt 6.5+ (online installer → `MSVC 2022 64-bit` component)
- OpenSSL 3 — easiest via Chocolatey: `choco install openssl`
- CMake 3.21+ (bundled with VS)

Build:
```pwsh
git clone <repo> nyx
cd nyx
cmake -S . -B build -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH="C:/Qt/6.7.3/msvc2019_64" `
  -DOPENSSL_ROOT_DIR="C:/Program Files/OpenSSL-Win64"
cmake --build build --config Release
.\build\nyx.exe
```

`windeployqt` runs as a post-build step — copies Qt DLLs + QML plugins next to `nyx.exe`.

## What Nyx does on first launch

1. Generates EC P-256 CA + leaf cert under `%APPDATA%\nyx\Nyx\`.
2. **Click "Install certificate"** in the UI. Adds the CA to `CurrentUser\Root` (no UAC).
3. Starts ConfigProxy HTTP server on a random localhost port.
4. Starts chat TLS proxy on `127.0.0.1:5223`.
5. **Click "Launch Riot Client"**. Nyx spawns `RiotClientServices.exe` with `--client-config-url=http://127.0.0.1:<port>`.

## Verify it works

Activity log should show, in order:
```
configproxy: listening on http://127.0.0.1:<port>
chat resolved: chat.<region>.lol.riotgames.com:5223
proxy listening on 127.0.0.1:5223, upstream chat.<region>.lol.riotgames.com:5223
client: incoming 127.0.0.1
client: TLS up — dialing upstream
upstream: TLS up
```

If you see those six lines, the pipeline is end-to-end working. Pick a mode in the UI, ask a friend on another account to look at your status.

## If something breaks

### `Could not load the Qt platform plugin "windows"`
`windeployqt` didn't run or `platforms\qwindows.dll` is missing. Re-run `windeployqt build\nyx.exe` manually.

### `tls backend: ...` log line shows `schannel`
The `QT_TLS_BACKEND=openssl` override didn't take. Either:
- OpenSSL DLLs (`libssl-3-x64.dll`, `libcrypto-3-x64.dll`) aren't next to `nyx.exe`
- Qt was built without the OpenSSL backend (rare on standard installs)

Fix: copy them from `C:\Program Files\OpenSSL-Win64\bin\` to `build\`.

### `RiotClientServices not found`
Launcher checks: `%ProgramData%`, `%ProgramFiles%`, `%LOCALAPPDATA%`, plus the legacy `C:\Riot Games` path. If Riot installed elsewhere, set the env var:
```pwsh
$env:NYX_RIOT_CLIENT = "D:\Games\Riot\Riot Client\RiotClientServices.exe"
```
(then re-launch nyx — currently not wired; add to launcher.cpp if needed)

### Riot Client launches but chat connection fails
Open Activity tab in Nyx. If `chat resolved: ...` never appears, your Riot Client didn't pick up the `--client-config-url` arg — verify Nyx is killing any pre-existing Riot Client processes before launch (Deceive does this; Nyx doesn't yet).

### CA not trusted by Riot Client
Confirm cert is installed:
```pwsh
certutil -store -user Root | Select-String "Nyx Local Root CA"
```
If missing, click "Install certificate" again in Nyx UI.

## Uninstall

In Nyx: Settings → Uninstall → "Remove Nyx". Removes:
- CA from `CurrentUser\Root`
- Cert files under `%APPDATA%\nyx\Nyx\`
- ConfigProxy + chat proxy stop on app quit

Riot Client returns to vanilla behavior on next launch (no Nyx-injected config URL).

## Anti-ban posture on Windows

- No process injection, no `RiotClientServices.exe` patching, no minhook
- No game traffic (UDP / WSS to game servers) is touched — only chat XMPP
- `clientconfig.rpg.riotgames.com` API responses are forwarded **and** modified per Riot's own `--client-config-url` flag (officially supported)
- Vanguard kernel driver only inspects the game process; chat path is invisible to it

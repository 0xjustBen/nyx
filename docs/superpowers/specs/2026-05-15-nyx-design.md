# Nyx — Design Spec

Date: 2026-05-15
Status: Approved (defaults — proceeding without further questions per session directive)

## Goal

Cross-platform LoL chat-state spoofer. Same technique as [Deceive](https://github.com/molenzwiebel/Deceive): local TLS MITM on the XMPP chat connection between Riot Client and `chat.<region>.lol.riotgames.com:5223`. Rewrite outbound `<presence>` stanzas so the user appears offline / invisible / mobile / away to friends, without touching the game client or game traffic.

Targets: Windows 10+, macOS 12+ (Apple Silicon + Intel), Linux (LoL-on-Wine).

## Non-goals

- DLL injection, memory patching, hooks into the game process
- Spectator-mode tricks, game-state spoofing
- Anything in the LoL game-server traffic path
- Friend-list manipulation beyond what XMPP presence allows

## Anti-ban posture

Riot's Vanguard / Riot Client only cares about game-process integrity and TLS auth to chat. The chat server treats us as a normal authenticated client. Because we:

- never inject into Riot processes
- never alter game traffic
- only rewrite `<presence>` stanzas in a stream we own (the client willingly speaks to `127.0.0.1` after `system.yaml` patch)
- install a CA into the user-scope trust store (not system)

…the detection vectors are limited to: (a) cert pinning, (b) `system.yaml` integrity check. (a) historically absent; (b) absent in current builds. Phase 0 smoke test confirms both before phase 1 begins.

## Architecture

```
┌───────────────────────────┐
│ Riot Client (LCU + chat)  │
└──────────────┬────────────┘
               │ XMPP-over-TLS to 127.0.0.1:5223
               │ (patched via system.yaml chat_host rewrite)
               ▼
┌───────────────────────────┐    ┌─────────────────────────────┐
│ Nyx proxy (QSslServer)    │◄──►│ Nyx XMPP rewriter           │
│  - leaf cert chained to   │    │  - QXmlStreamReader streaming│
│    Nyx CA (in user trust) │    │  - presence/show/status     │
│  - one upstream per conn  │    │  - strip game extension     │
└──────────────┬────────────┘    └─────────────────────────────┘
               │ XMPP-over-TLS to real chat.<region>.lol.riotgames.com:5223
               ▼
┌───────────────────────────┐
│ Riot chat server (XMPP)   │
└───────────────────────────┘
```

Single process. UI thread = Qt main loop. Proxy on dedicated thread or async on main via Qt sockets (decision: async on main — `QSslServer` is non-blocking, no thread needed).

## Components

| Module | Responsibility |
|---|---|
| `core/cert` | EC P-256 root CA + per-host leaf, save PEM under app data dir |
| `platform/trust_{mac,win,linux}` | Install CA into **user-scope** trust store; Wine registry path for Linux |
| `core/patcher` | Rewrite `chat_host:` in `system.yaml`; backup as `.nyx-bak`; restore on uninstall |
| `core/riot_paths` | Locate Riot install per OS (incl. Wine prefix) |
| `core/proxy` | `QSslServer` on 127.0.0.1:5223, per-connection upstream `QSslSocket` |
| `core/xmpp` | `QXmlStreamReader` streaming rewriter; Mode enum drives transforms |
| `core/config` | INI via `QSettings`; mode, region, autostart |
| `ui/app_controller` | Qt model exposed to QML; orchestrates lifecycle |
| `ui/roster_model` | `QAbstractListModel` of friends populated from incoming roster IQ |
| `qml/*` | Frameless on Win/Linux, native chrome on macOS; sidebar nav; 3 screens |

## Modes

| Mode | C→S transform | Semantics |
|---|---|---|
| online | passthrough | normal |
| away | rewrite `<show>` → `away`, strip game ext | friends see Away |
| mobile (default) | rewrite `<show>` → `mobile`, set client `<c node="riot-mobile">`, strip game ext | friends see Mobile, no game-in-progress |
| invisible | drop outbound presence broadcasts entirely; reply `<presence type="unavailable"/>` to roster queries | friends see Offline; you still get inbound chat |
| offline | same as invisible but also suppress sending chat-message presence receipts | strongest hide |

Inbound (S→C) is always passthrough — we need real roster + friend presence to render UI.

## Data flow per connection

1. Riot Client opens TLS to `127.0.0.1:5223`.
2. `QSslServer` accepts, presents leaf cert signed by Nyx CA. Client trusts (CA in user store).
3. On `encrypted()`, Nyx opens upstream `QSslSocket` to real chat host (looked up from pre-patch backup of `system.yaml`).
4. Bytes pumped both ways. Each direction has its own `QXmlStreamReader` for stanza detection.
5. On `<presence>` from client side, rewriter applies current Mode, emits possibly-modified XML to upstream socket. Server side is verbatim.

## Error handling

- Upstream TLS failure → log, close client connection (Riot retries → falls back to normal flow if CA absent, or fails closed if patched but Nyx down).
- Bad stanza (XML parse error) → flush remaining buffer verbatim, do not block client.
- Port 5223 in use → fail start, surface error in UI with "another Nyx instance?" hint.
- `system.yaml` write permission denied → UI error, retain non-patched state.

## Testing

- **Phase 0 smoke**: manual mitmproxy + curl-against-localhost validates trust + XMPP semantics work end-to-end before any Nyx code in proxy/xmpp paths.
- Unit tests: `xmpp` rewriter with golden-vector stanzas (one per mode × stanza-type).
- Integration: spin up a local fake XMPP server (`prosody` in docker) + Nyx proxy + xmpp test client. Run on CI Linux only.
- Manual: real Riot Client on each OS before each release.

## Risks

| Risk | Mitigation |
|---|---|
| Riot adds chat cert pinning | Phase 0 detects; project dead-ends if so — surface clearly |
| `system.yaml` signature check | Phase 0 detects; fallback path = hosts file (v2) |
| Wine prefix variance on Linux | Document `WINEPREFIX` env; UI input field |
| Trust install requires admin on Win | Use `CurrentUser\Root` (no UAC) |
| User chains Nyx with Vanguard kernel checks | Vanguard does not inspect chat path; verify in QA |

## Out of scope (v1)

- Anti-screenshare (Discord overlay leakage of "in game")
- Auto-update
- Multi-account
- Game-mode-specific overrides
- Telemetry

## Dependencies

- Qt 6.5+ (Core Gui Qml Quick QuickControls2 Network Widgets)
- OpenSSL 3 (macOS/Linux; Qt-bundled on Windows)
- CMake 3.21+
- C++17

No asio, no pugixml.

## Phase plan

0. Smoke test (manual, no code).
1. Cert gen + user-scope trust install.
2. Proxy (byte-pump only, no rewriting).
3. system.yaml patcher.
4. XMPP streaming rewriter + modes.
5. UI polish + tray + autostart.
6. Installers (MSI / DMG / AppImage).

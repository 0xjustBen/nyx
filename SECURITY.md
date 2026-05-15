# Security

## Threat model

Nyx generates a per-user root CA and installs it into the user-scope trust
store. Anyone with read access to that key can mint TLS certs trusted by
*this user's* Riot Client. They cannot mint certs trusted by anyone else.

## Key storage

- Linux/macOS: `~/.local/share/Nyx/ca.key` (0600 mode)
- Windows: `%APPDATA%\nyx\Nyx\ca.key` + DPAPI-sealed copy `ca.key.dpapi`.
  The sealed copy is bound to the current Windows user and cannot be
  decrypted from another account on the same machine.

## Network surface

- ConfigProxy HTTP server: bound to `127.0.0.1` only, random port. No
  authentication — anyone on the same machine can hit it. Mitigated by
  the localhost-only bind and by upstream requiring valid Riot JWTs.
- Chat TLS proxy: bound to `127.0.0.1`/`::1`, random port. Server cert is
  scoped to the localhost-domain SAN. Upstream connection to the real
  Riot chat server uses `VerifyPeer` against the system trust store.

## What Nyx does NOT do

- No process injection into Riot Client / League / Valorant
- No kernel-mode hooks; invisible to Vanguard
- No modification of game files or memory
- No network telemetry — local-only

## Reporting issues

Open a GitHub issue with `security:` prefix. For sensitive disclosures,
encrypt to the maintainer's public key (TBD).

# Phase 0 — Smoke test

Purpose: prove the technique still works before writing Nyx code. Confirms (a) no chat cert pinning, (b) no `system.yaml` signature check.

Run on a throwaway Riot account. Estimated time: 30-60 min.

## Tools

- `mitmproxy` (`brew install mitmproxy`)
- a hex/text editor
- LoL installed and logged-in working baseline

## Steps

### 1. Baseline capture

1. Launch Riot Client → log in → exit.
2. Locate `system.yaml`:
   - macOS: `/Applications/League of Legends.app/Contents/LoL/system.yaml`
   - Win:   `C:\Riot Games\League of Legends\system.yaml`
3. Copy aside as `system.yaml.orig`.
4. Find the `chat_host:` key under each region (e.g. `chat.na1.lol.riotgames.com`). Note value.

### 2. Trust mitmproxy CA (user scope)

1. Run `mitmproxy` once to generate `~/.mitmproxy/mitmproxy-ca-cert.pem`.
2. Install **user-scope only**:
   - macOS: Keychain Access → File → Import → `mitmproxy-ca-cert.pem` into **login** keychain → set Trust to "Always Trust". **Do not** drag into System keychain.
   - Win: `certmgr.msc` → Current User → Trusted Root Certification Authorities → Import.
   - Linux/Wine: `wine regedit` → `HKLM\Software\Microsoft\SystemCertificates\ROOT\Certificates\<thumb>` (or use `wine certutil -addstore`).

### 3. Redirect chat to local mitmproxy

Edit `system.yaml`:
```yaml
chat_host: "127.0.0.1"
chat_port: 5223
```
(repeat for whichever region you log in to)

### 4. Run mitmproxy as raw TCP listener

XMPP-over-TLS is not HTTP. Use mitmproxy's reverse-mode TLS pipe:

```
mitmdump --mode reverse:tls://chat.<region>.lol.riotgames.com:5223 \
         --listen-host 127.0.0.1 --listen-port 5223 \
         --set tls=true --ssl-insecure
```

(The CA mitmproxy uses to mint a `chat.*.lol.riotgames.com` leaf must be the one you trusted in step 2.)

### 5. Launch Riot Client, log in, watch traffic

Expected: client connects through mitmproxy without TLS errors, friends list populates, chat works normally.

**Pass criteria**: roster IQ + presence stanzas visible in mitmproxy log; no client-side TLS / pinning error; friend can see your online status.

### 6. Try presence rewrite (manual)

Add a tiny mitmproxy addon `rewrite.py` that intercepts the next outbound `<presence>` stanza and changes `<show>chat</show>` → `<show>mobile</show>`. Verify friend sees you as Mobile.

### 7. Restore

1. Quit Riot Client.
2. Copy `system.yaml.orig` back over `system.yaml`.
3. Remove mitmproxy CA from your user trust store.

## Outcomes

- **All pass** → green light Phase 1. Document any quirks (e.g. region-specific behavior) here.
- **Cert pinning blocks connection** → project dead-end on Riot's current build; revisit if Riot ever publishes a workaround.
- **system.yaml signature check fails** → Phase 1 still viable, but Phase 3 must use hosts-file fallback. Update spec.

## Notes

- Riot logs are at `~/Library/Logs/LeagueOfLegends/` (macOS) / `%LOCALAPPDATA%\Riot Games\Riot Client\Logs\` (Win). Check `Riot Client UX Logs` for any "certificate" / "pinning" complaints.
- If a future LoL update breaks Phase 0, do not ship — revisit pinning/signature status.

# Using go_fish

go_fish replaces the default macOS Cmd+Tab application switcher with a grid
of window thumbnails — every window of every regular app, including
minimized ones, each addressable individually.

## First run

After `./build.sh` produces `./go_fish.app` in the repo root, launch it:

```sh
open go_fish.app
```

(Or double-click `go_fish.app` in Finder.) `build.sh` does **not**
register go_fish for startup — it runs as a menu-bar app you start
yourself. Auto-launch on login is opt-in via the **Start at boot** menu
item once go_fish is up.

On the first launch, go_fish needs two permissions and will exit with a
message after each. Grant them in **System Settings → Privacy & Security**:

1. **Accessibility** — required to read each app's window list, raise
   windows, and un-minimize them.
2. **Screen Recording** — required to capture live thumbnails of windows.
   Without it, every tile falls back to the app icon (the program still
   works, just less informative).

If you launch before granting permissions, the binary will exit and
write the failure to its attempt counter at
`~/Library/Application Support/go_fish/attempts.txt`. The next two
launches add a 1 s and 2 s sleep before retrying; after a 3rd failed
preflight, the binary exits *cleanly* (`exit 0`) and resets the counter —
so when launched as a Login Item, a missing permission re-prompts at most
three times across logins instead of on every single one. A successful
start also resets the counter.

After granting both, re-launch. You should see:

```
go_fish running. Press Cmd+Tab to switch windows, or click the hook in the menu bar.
```

A fishing-hook icon appears in the right side of the menu bar while
go_fish is running. Click it to drop down a menu:

- **Show Window Grid** — opens the all-apps grid (same as Cmd+Tab).
- **Minimize All** — minimizes every standard window of every regular
  app. Windows already minimized are skipped.
- **Cascade All** — un-minimizes minimized windows, resizes each to
  ~75% of the visible screen area (clamped to 480×320 / 1600×1000), and
  arranges them in a staircase on the screen under the cursor. The
  cascade runs back-to-front, so the bottom-most window lands at the
  top-left and every window's title bar stays visible. Apps that refuse
  AX position writes (fixed-UI Electron tools, full-screen apps that
  don't expose `AXFullScreen`, etc.) are skipped silently; the log file
  (`~/Library/Logs/go_fish.err.log`) records which.
- **About…** — a small window with the app version and the date the
  running binary was compiled. The GPL 3.0 licence and `http://camb.io`
  links open in your default browser.
- **Start at boot** (toggle) — adds / removes the `go_fish.app` bundle in
  your per-user **Login Items** (System Settings → General → Login Items).
  Effective on next login; the currently running instance is not
  affected. Because a bundle is registered (not the bare binary), it
  launches at login with **no Terminal window**. See **Auto-launch on
  login** below for the contract details.
- **Secure Event Input detection** (toggle) — when checked (default),
  go_fish polls `IsSecureEventInputEnabled()` every 0.5 s. While Secure
  Event Input is held by another app, the menu-bar icon gets a red X
  overlay and the tooltip changes to "Secure Event Input is active
  (go_fish shortcuts still work)". go_fish itself is unaffected; the
  indicator shows when other tools that rely on event taps go blind.
- **Quit** — terminates go_fish. The keyboard shortcut listed beside it
  (⌘Q) only works while the menu is open; there's no key window
  otherwise.

## The system's own Cmd+Tab and Cmd+\`

macOS's Cmd+Tab, Cmd+Shift+Tab (the Dock's app switcher) and Cmd+\` ("Move
focus to next window") would win over go_fish's hotkeys. With the Cmd
hotkey modifier, go_fish switches all three off while it runs, so there is
nothing to change in System Settings. When go_fish quits it puts back
whatever System Settings has saved for each one, so a shortcut you turned
off there stays off.

If go_fish crashes or is killed with `kill -9`, the three stay off until
go_fish runs and quits again (or you log out).

## Using the switcher

| Action                          | Result                                             |
| ------------------------------- | -------------------------------------------------- |
| **Cmd + Tab**                   | Open the grid with every app's windows; pre-select the next-most-recent |
| **Cmd + \`**                    | Open the grid filtered to the **current app**'s windows only |
| **Click the menu-bar hook → "Show Window Grid"** | Open the grid (all apps; no Cmd needed; commit by clicking a tile) |
| **Tab** (while Cmd held)        | Cycle forward                                      |
| **Shift + Tab** (while grid is open) | Cycle backward                                |
| **Move mouse over a tile**      | Hover-select: the selection follows the cursor     |
| **Click a tile**                | Select that tile and commit immediately            |
| **Click the X on a tile**       | Close that window (AX-press its close button); the tile drops out of the grid without dismissing the panel |
| **Release Cmd**                 | Commit: focus the selected window                  |
| **Escape**                      | Cancel: close the grid, do nothing                 |

Cmd+\` always shows the grid, even if the current app has just one
window — useful for confirming what's open without context-switching.

Keyboard and mouse can be mixed freely: hover changes the selection that
Cmd-release would commit to, and a click bypasses the Cmd-release step
entirely (so you don't even need to be holding Cmd anymore once the grid
is up).

When you commit:

- If the chosen window is **minimized**, go_fish un-minimizes it (restoring
  it to its last size and position) and brings it to the front.
- If the chosen window is on **another Space or another display**, macOS
  automatically switches to that Space as a side-effect of activating the
  owning app.
- If the chosen window is **already frontmost**, nothing visible happens —
  same as native Cmd+Tab.

## What appears in the grid

Every "standard" window of every running app whose activation policy is
*regular* (i.e. apps that show up in the Dock). This includes:

- Visible windows on the current Space
- Windows on other Spaces / other displays
- Minimized windows (shown with their app icon and a small "minimized"
  badge, since no live image is available)
- One placeholder per **unresponsive app** — apps that don't reply to
  the AX windows query within 100 ms. The placeholder uses the app icon,
  shows a red "not responding" badge, dims the tile to 70%, and omits
  the X button (we can't reliably close a specific window through an
  app that isn't talking back). Clicking the tile still activates the
  app via `NSRunningApplication`, which usually unsticks it.

It excludes palettes, system dialogs, menu-bar–only apps, the Dock, etc.

### Tile anatomy

Each tile shows, top to bottom:

- The **app name** (bold, centered), with an **X close button** to its
  left in the same horizontal band.
- The **thumbnail** (live capture for visible windows; app icon for
  minimized or just-snapshotted ones). A "minimized" or "not responding"
  badge overlays the top-left when applicable.
- The **window title** (or the app name as a fallback when the window
  has no title).

The selected tile is wrapped in a translucent accent-colored highlight
that follows the keyboard or mouse selection.

Windows are ordered **most-recently-used first**, so:

- Position 1 (the one pre-selected when the grid opens) is always the window
  you were on just before the current one. A quick Cmd+Tab — press and
  release without cycling — toggles between the last two windows you used.
- Repeated quick Cmd+Tabs ping-pong between those two, the same way native
  Cmd+Tab does for apps.
- Windows you've focused at any point during this go_fish session are
  ordered by recency; windows that haven't been touched fall back to
  z-order, then off-Space, then minimized at the very end.

MRU is rebuilt in memory each time go_fish starts (seeded from the current
z-order) and updated by listening to `NSWorkspaceDidActivateApplication`
notifications plus per-app `kAXFocusedWindowChangedNotification`
observers, so within-app window switches (clicking another window,
in-app Cmd+\`, etc.) also register.

## Configuration

The hotkey modifier (Cmd or Ctrl) is in **Settings…**. Other configuration
points live in the source:

| Setting              | Where to change in source                  | Default                  |
| -------------------- | ------------------------------------------ | ------------------------ |
| Trigger keys         | `cocoa.m` → `gf_switch_hotkeys`            | Tab, \`                  |
| Tile size            | `cocoa.m` → `gf_preferredPanelSize` `tileW`| 240pt wide tiles         |
| Panel clamp          | `cocoa.m` → `gf_showPanel` `maxW`/`maxH`   | 90% × 85% of screen      |
| Per-app AX timeout   | `cocoa.m` → `kAXAppTimeout`                | 0.1 s                    |
| Thumbnail cache cap  | `cocoa.m` → `kThumbCap`                    | 30 entries               |
| Thumbnail stale-after| `cocoa.m` → `kThumbStaleAfter`             | 30 s                     |
| Cascade offset       | `cocoa.m` → `gf_cascadeAll` `offset`       | 32pt                     |
| Cascade target size  | `cocoa.m` → `gf_cascadeAll` `targetW/H`    | 75% of vf, clamped       |

To rebuild after editing, see `BUILDING.md`.

## Running in the background, always

go_fish runs as an accessory app (no Dock icon by default; only the
menu-bar hook) and idles at effectively zero CPU until you press the
hotkey.

### Build

```sh
./build.sh                # compile ./src → ./go_fish.app (needs Xcode CLT)
open go_fish.app          # launch it
```

`build.sh` compiles the sources, generates the icon, assembles
`./go_fish.app`, and ad-hoc signs it. It does **not** register go_fish for
startup or move the bundle anywhere — launch it yourself with `open
go_fish.app` (or double-click in Finder), and move the bundle wherever you
like (e.g. `~/Applications`); it's self-contained. To uninstall, quit it
and drag the `.app` to the Trash; remove the **Start at boot** entry first
if you enabled it (uncheck it in the menu, or delete it under System
Settings → General → Login Items).

### Auto-launch on login: Start at boot

Open the menu-bar hook and check **Start at boot**. That adds the
`go_fish.app` bundle to your per-user **Login Items** (the list under
**System Settings → General → Login Items**, the same one the "+" button
populates), via the `LSSharedFileList` session list. Contract:

- **No effect on the currently running instance.** The entry takes effect
  on your next login; the current process keeps running.
- **No Terminal window at login.** The login item points at the `.app`
  bundle, which LaunchServices runs directly. (A bare Unix binary in
  Login Items has no opener, so macOS hosts it in Terminal — the stray
  window older builds showed. If you enabled Start at boot under one of
  those builds, re-toggle it once after upgrading so the entry repoints
  at the bundle.)
- **Matched by resolved path.** The toggle reflects whether *this*
  install is registered. A stale entry pointing at a different location
  (e.g. an old dev-build path) reads as not installed, so the checkmark
  tracks the app you're actually running.
- **Unchecking** removes that entry from the list.
- **Launches once at login.** Unlike the old LaunchAgent, a Login Item is
  not auto-restarted if it crashes. A permission-prompt guard still
  applies: the binary tracks attempts in
  `~/Library/Application Support/go_fish/attempts.txt`, sleeping 0 s, 1 s,
  then 2 s before consecutive permission preflights, and after the 3rd
  failure exits with `0` and resets the counter — so a missing permission
  can't re-prompt on every login.

You can inspect or edit the same list from the command line:

```sh
# list current login items
osascript -e 'tell application "System Events" to get the name of every login item'

# remove go_fish's entry by hand
osascript -e 'tell application "System Events" to delete (every login item whose name is "go_fish")'
```

> Earlier versions registered a LaunchAgent at
> `~/Library/LaunchAgents/com.local.gofish.plist` instead. If you ran one
> of those, remove that legacy plist by hand:
> `rm ~/Library/LaunchAgents/com.local.gofish.plist`.

## Resource footprint

- Idle: a few tens of MB resident, ~0% CPU. The only ongoing background
  work is registered hotkeys waiting on the run loop, plus
  occasional thumbnail re-captures triggered by focus/activation events
  (a single `CGWindowListCreateImage` per event, ~10–30 ms each).
- Per activation: one round of AX enumeration plus, if the cache is hit,
  zero capture work — thumbnails appear instantly.
- A thumbnail cache holds up to 30 recently-focused windows (LRU,
  ~25–30 MB at the cap). It's pre-warmed at startup (visible windows are
  captured serially over a few seconds at utility priority) and refreshed
  whenever a window gets focused, an app gets activated, or you activate
  a window via go_fish itself. Cached entries older than 30 s get
  re-captured in the background when the grid opens — the stale version
  is still displayed instantly while the refresh runs.
- Window list is **not** maintained in memory between activations; it's
  snapshotted on demand.

## Troubleshooting

### Cmd+Tab still opens the system switcher

WindowServer's built-in shortcut is taking the key before go_fish sees it.
Disable it (see "Disable the system Cmd+Tab" above).

### Panel doesn't appear, but the system one is disabled

- Confirm Accessibility permission is granted (System Settings → Privacy
  & Security → Accessibility). The toggle must be **on** for `go_fish`.
- If you re-signed or rebuilt the binary, macOS may consider it a new app
  and silently revoke the permission until you re-grant it.
- Check the binary's stderr. Launched as a Login Item or via `open` it
  goes to the system log (`log stream --predicate 'process ==
  "go_fish"'`). To capture it to a file, run the bundle's executable
  directly from a terminal, e.g.
  `~/Applications/go_fish.app/Contents/MacOS/go_fish >~/Library/Logs/go_fish.err.log 2>&1`.

### Cmd+Tab silently does nothing (system switcher also doesn't appear)

Most likely a hotkey failed to register. Another app may already own
the same shortcut. Run the bundle's executable from a terminal and look
for `could not register hotkey`:

```sh
pkill -x go_fish; ~/Applications/go_fish.app/Contents/MacOS/go_fish
```

When the Cmd hotkeys don't all register, go_fish leaves the system's
Cmd+Tab switched on, so the Dock's switcher appears instead.

### Thumbnails are missing / all entries show app icons

Grant Screen Recording permission. After granting, you must quit and
re-launch go_fish — macOS only re-checks the permission at process start.

### A window appears in the grid that I can't activate

Some sandboxed apps return AX window elements that don't accept
`kAXRaiseAction`. go_fish will still call `activateWithOptions:` on the
owning app, which usually brings *some* window of that app forward but may
not be the exact one you selected. Known offender: certain Electron apps
in unusual states.

### An app shows up as "not responding"

The app didn't reply to go_fish's AX windows query within the per-app
budget (100 ms by default — see `kAXAppTimeout` in `cocoa.m`). The most
common causes are an app stuck waiting on disk/network, mid-launch
initialization, or an in-progress modal that hasn't fully realized. The
placeholder tile lets you still bring the app forward — clicking it
calls `activateWithOptions:` on the process, which often unsticks the
event loop. If you want to be more aggressive: right-click the Dock icon
and choose Force Quit, or `kill <pid>` from a shell.

### Cascade All didn't move some windows

`Cascade All` writes `kAXPositionAttribute` on each window, but some apps
refuse position writes (fixed-UI tools, full-screen apps that don't
expose `AXFullScreen`, certain Electron apps). Skipped windows are
logged to `~/Library/Logs/go_fish.err.log` with their app name and the
AX error code; a summary line at the end reports `moved N, resized M,
skipped K of T`. Full-screen apps are best-effort un-fullscreened first
(via the undocumented `AXFullScreen` attribute) — works for Safari,
Mail, most Apple apps; not for every third-party.

### After granting permissions, it still complains

Permissions are tied to the binary's signing identity. For an ad-hoc
signed binary (what `build.sh` produces by default), that identity is
the **CDHash** — a hash of the binary's bytes. Every rebuild produces
different bytes, so a fresh CDHash, so macOS treats it as a new app and
re-prompts.

The System Settings UI shows one entry per path, so you'll see the
go_fish row still toggled **on** — but that's the previous CDHash's
grant. The OS is waiting on permission for the current binary.

**To fix once:** in **System Settings → Privacy & Security → Accessibility**,
select the go_fish row and click the **−** button to remove it entirely.
Do the same under **Screen Recording**. Re-run `./build.sh` and relaunch.
The next prompt will create a fresh entry that matches the running binary,
and it'll stick until the next rebuild.

**To stop the cycle:** stop using ad-hoc signing. Either
(a) build once and don't rebuild, or
(b) sign with a stable identity (Developer ID, or a self-signed code-
signing cert you create in Keychain Access). See `BUILDING.md` →
*Code signing* for both options.

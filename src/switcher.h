#pragma once

// Switcher state machine — the event-driven core that owns the window list,
// the selected index, and the open/closed state. Formerly switcher.go; these
// functions were the cgo //export surface and are now plain C, called directly
// by the hotkey handler and panel UI in cocoa.m.
//
// All are invoked on the main thread (the hotkey handler, the release poll and
// the panel mouse handlers all run on the main run loop). They guard the shared state
// with an internal mutex regardless, matching the original's contract.
//
// None of them block on window enumeration: gfOnHotkey starts the snapshot and
// returns, and the grid appears when the list lands. Presses, a hotkey release,
// or a cancel arriving before then are held and applied on arrival.
//
//   shift: nonzero reverses cycle direction.
//   scope: 0 = all regular apps (Cmd+Tab), 1 = frontmost app only (Cmd+`).
//
// gfOnHotkey / gfOnCommit / gfOnCancel / gfOnClose return 1 if they handled
// the event and 0 otherwise.

int  gfOnHotkey(int shift, int scope);
int  gfOnCommit(void);
int  gfOnCancel(void);
void gfSetSelection(int idx);
int  gfOnClose(int idx);
void gfOnFocus(int idx);
// Re-sort the live window list by the current sort mode and refresh the panel.
// Called when the user clicks the sort toggle in the grid.
void gfToggleSort(void);

# Players and scores — custom v1.3

## Using it
- Tap the name above a life total to choose a saved player. Names rotate with each seat in 2/3/4-player modes. The initial labels remain Player 1, Player 2, etc. until you select people.
- Preloaded profiles: Steve, Chris, Marvin, Marian, Erwin, Max, Fabi, Duki, David, Marcel, Emil. Initial wins: zero. Existing profile saves take precedence on future boots.
- Five names per page. Previous/Next browse the list. A person cannot occupy two active seats. Guest clears the seat assignment and uses its Player N label.
- New player opens an on-screen keyboard. Maximum 32 saved profiles; names are 1–16 characters, using Latin letters, digits, spaces, apostrophe or hyphen. Case-insensitive duplicates are rejected. Leading/trailing spaces are trimmed. Cancel discards typing.
- Settings > Players & scores shows names and lifetime wins. Tap a name to rename it; wins and seat assignments follow that profile.
- Settings > Reset asks Who won this game? Choose an active seated player to add one win and start a fresh game. Choose a guest or No winner / reset to reset without a win. Cancel returns to Settings without resetting.
- Reset retains seats and scores. Changing player mode/starting life keeps the existing immediate-reset behaviour and does not award a win.
- Players & scores > Undo last win asks for confirmation and removes only the latest recorded win, once. It does not alter the current game. No delete/clear-all action is added.
- Physical roll buttons are ignored while a player dialog is open, avoiding accidental dismissal. Normal game gestures, history and dice remain available outside dialogs.

## Saving and compatibility
The firmware stores profiles, seats, scores, undo record, game, history and dice choice together in the versioned session_v1 NVS blob. Profile edits, seating changes, results, undo and resets trigger a save in the main loop; sleep also saves. Writes precede the pending display refresh. Failed writes retain the dirty flag and retry after five seconds, logging the error.
Existing firmware saves are read through the previous game/history/dice keys when no session_v1 exists. The old keys are left intact for rollback; a rollback would resume that older saved state, not the new profiles or latest game. Firmware flashing must preserve NVS.
The firmware uses static session buffers to avoid adding a large structure to the small main task stack. Profiles are bounded, and snapshots validate names, count, unique seats, IDs and undo state before restoring.

## Code map
- core/include/sticky_lotus/PlayerProfiles.h: seed names, storage limits, names, seats, wins and validation.
- engine/src/ProfileActions.cpp: player selection, keyboard input, reset winner prompt and undo.
- engine/src/ProfileRenderer.cpp: dialog layouts.
- engine/include/sticky_lotus/ui/AppRenderer.h: shared hit boxes and player-name placement.
- engine/src/AppRenderer.cpp: Settings v1.3 and names on full/regional game redraws.
- firmware/main/sticky_lotus_firmware.cpp: NVS migration, session saving and retry.

## Validation
Headless and graphical integration tests cover the eleven seeds, capacity and name validation, duplicate seating, moving an inactive seat, selection without life changes, name rotation in all modes, regional life redraw, keyboard creation/rename, pagination, guests, winner/reset/cancel, repeated release, hardware roll suppression, undo, profile restore and corrupt snapshot rejection. Existing gameplay and refresh regression suites also pass. Screenshots were visually inspected.
Firmware compilation is blocked in the agent environment by the ESP toolchain launcher returning Windows error 5. Run the packaged Build-firmware.ps1 in a normal PowerShell window. It verifies existing artwork/history/dice/refresh markers plus profile screens, names, v1.3, the new storage key and individual stack frames. Actual NVS migration, power-cycle persistence and touch use still require device testing after that build and installation.

## First hardware build check
The first v1.3 build was rejected before flashing: drawProfiles used a 1760-byte frame (limit 768). Other measured frames included handleProfileInput 288 bytes and saveGameState 432 bytes. The renderer now avoids implicit temporary strings and reuses one 64-byte label buffer. A new firmware build must pass the same unchanged stack limit before installation.

The second hardware build measured 1136 bytes for the combined renderer and was also rejected before flashing. Each dialog now has a separate non-inlined drawing function, preventing the compiler from reserving argument temporaries for all layouts together. The unchanged 768-byte frame limit now checks every dialog helper as well as the dispatcher.

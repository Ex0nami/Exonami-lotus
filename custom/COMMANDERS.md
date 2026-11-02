# Own and partner commanders

Each seat defaults to a single commander. Tap its name, then use the Partner commanders ON/OFF toggle; no startup question is added. Configuration belongs to the seat, not a saved person, and survives game reset and sleep.

Swipe sideways to open incoming commander damage. All player areas are editable, including your own (for example if another player controls your commander). Partner sources have A/B selectors; choosing one does not add damage. Minus/plus and holds operate on the selected commander. Swiping sideways commits both counters and updates life once; cancel discards both. Each individual commander has its own 21-damage threshold; A and B never combine for elimination. History identifies A/B for partner entries.

Disabling partners is blocked while B has damage recorded against any seat. Correct that damage to zero or reset the game first, so lethal damage is never silently hidden. Reset clears damage but keeps the seat configuration. Switching player modes also clears damage under the existing reset behavior.

## Persistence

GameSnapshotV1 preserves the old serialized layout. New session_v2 saves add the four partner flags and sixteen B counters. Loading prefers v2, then migrates session_v1 with old damage mapped to A and all partners off; older profile-free saves are also converted. Names, scores, history and dice are retained. Old keys remain untouched; downgrading reads the old saved state rather than the latest game. Installation must preserve NVS.

## Verification

The new integration suite covers own damage and elimination, separate partner limits, correction/revival, toggle defaults, snapshot validation and legacy conversion, A/B history, draft cancel/commit, reset, and touch selection for each seat in 2/3/4-player layouts. It runs alongside the existing gameplay/profile/refresh regressions. Graphical screenshots cover all layouts.

Firmware build in the agent environment remains blocked by Windows toolchain launch errors. The separate commanders build helper runs existing firmware checks plus verify-commanders-firmware.py before packaging. Hardware validation and a real-device migration test remain pending. The battery-latch startup fix remains in place.

# Changelog

## 1.4 — 2026-11-02

### What's new

- **Own commander damage:** record damage from your own commander, including when another player controls it.
- **Partner commanders:** toggle partners in the player-name menu. Track commanders A and B separately, each with its own 21-damage limit. Single commander is the default, with no extra prompt.
- **Move players between seats:** selecting an already-seated player moves them directly to the chosen seat. The previous seat becomes its numbered guest placeholder; players are not swapped. Wins stay with the profile; life totals and commander settings stay with the seat.
- **Battery startup fix:** enable the power latch on every startup, fixing the observed freeze when USB was disconnected.

### Saved games

Existing names, scores, history and game data migrate to the new save format. Existing commander damage becomes commander A's damage. Update without erasing NVS to retain saved data. Turning partners off is blocked while commander B has recorded damage; correct it to zero or reset the game first.

### Validation

Desktop regression tests passed. The feature build passed firmware checks and was installed with verified startup, power-latch initialization and saved-game restoration. The 1.4 version labels were added afterward, so that installed image is not an exact build of the 1.4 release source. No new 1.4 firmware binary is included yet; device testing of the new interactions remains to be confirmed.

## Battery-power fix - 2026-10-26

Enabled the battery power latch at startup and wake, fixing the observed freeze when USB was disconnected. Saved games remain intact. Included in 1.4.

[Source milestone](https://github.com/Ex0nami/Exonami-lotus/commit/a0109f6).

## 1.3 - 2026-10-07

Added saved player names, selection and an on-screen keyboard; win scores, winner-on-reset, no-winner reset and undo last win. Saved profiles and game state together. Removed the Settings image and reduced player-menu stack use. This historical build predates the battery-power fix.

[Source milestone](https://github.com/Ex0nami/Exonami-lotus/commit/16d9b59).

## Refresh and performance - 2026-09-20

Added manual screen cleanup and automatic full refresh scheduling. Batched rapid updates, avoided redundant drawing, optimized monochrome fills and limited diagnostics.

[Source milestone](https://github.com/Ex0nami/Exonami-lotus/commit/845340f).

## Dice reset fix - 2026-09-18

Fixed excessive stack use that could restart the device after rolling a die or flipping a coin. Verified changed life totals survive both results.

[Source milestone](https://github.com/Ex0nami/Exonami-lotus/commit/b871dc0).

## Action history - 2026-09-17

Added the last ten actions, five per page. Long-press the centre icon for history; tap still opens Settings. History survives sleep.

[Source milestone](https://github.com/Ex0nami/Exonami-lotus/commit/f3d26a5).

## Centre menu icon - 2026-09-17

Added the Planeswalker icon and fixed sections of its surrounding circle disappearing after player updates.

[Source milestone](https://github.com/Ex0nami/Exonami-lotus/commit/53abbc3).

## Dice and coin - 2026-09-17

Added D6, D20 and coin flip on the middle hardware button: short press rolls, hold selects the mode. First-player roll stays on the bottom button. This initial build had a reset bug fixed September 18; it is not recommended for installation.

[Source milestone](https://github.com/Ex0nami/Exonami-lotus/commit/791575c).

## Three-player mode - 2026-09-16

Added one player across the top and two below, with appropriately oriented controls and text.

[Source milestone](https://github.com/Ex0nami/Exonami-lotus/commit/93aec63).

## Ping undo - 2026-09-16

Swipe up in the same seat within four seconds to undo the latest ping. Afterward swipe up opens poison. No temporary undo hint.

[Source milestone](https://github.com/Ex0nami/Exonami-lotus/commit/96fc0f3).

## Sleep artwork rotation - 2026-09-15

Rotated the custom sleep artwork 90 degrees counterclockwise.

[Source milestone](https://github.com/Ex0nami/Exonami-lotus/commit/6280332).

## Custom sleep artwork - 2026-09-15

Added Exonami artwork and five mana symbols, and connected it to the physical sleep-button flow.

[Source milestone](https://github.com/Ex0nami/Exonami-lotus/commit/475789d).

## Remove remaining arrows - 2026-09-15

Removed the remaining gesture arrows from the life screen while preserving poison and Commander functions.

[Source milestone](https://github.com/Ex0nami/Exonami-lotus/commit/225ad70).

## Cleaner screen and ping - 2026-09-15

Removed poison/Commander hint text from the life screen. Added player-relative swipe down to remove one life from other eligible players, with gesture regression tests.

[Source milestone](https://github.com/Ex0nami/Exonami-lotus/commit/a95e37b).

## Initial custom build - 2026-09-15

Set cumulative signed life changes to expire after four seconds. Kept upstream first-player selection, fixed Windows simulator compilation, matched hold gestures and added build instructions and desktop integration tests.

[Source milestone](https://github.com/Ex0nami/Exonami-lotus/commit/b7d35db).

## Upstream base and version numbering

Customization started from inkOne/sticky-lotus commit `1e5dc875` on September 15, 2026. The inherited tags `v1.0.1`, `v1.1.0` and `v1.1.1` belong to upstream, not Exonami edition. Earlier custom updates were unnumbered, so they are listed by date and feature rather than given invented version numbers. Internal test builds and documentation-only changes are grouped with their feature. Older builds are superseded; use the current version for new installations.

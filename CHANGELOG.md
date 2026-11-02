# Changelog

## 1.4 — 2026-11-02

- Track damage from your own commander.
- Enable partner commanders in the player-name menu, with separate A/B damage counters. Single commander remains the default.
- Move a saved player directly to another seat; the previous seat becomes its numbered guest placeholder.
- Preserve existing names, scores and games when upgrading without erasing saved data.

[Source milestone](https://github.com/Ex0nami/Exonami-lotus/commit/55a2ec7).

## 1.3.1 - 2026-10-26

- Fix freezing when USB is disconnected by enabling the battery power latch at startup and wake.
- Keep saved names, scores and game data intact.

[Source milestone](https://github.com/Ex0nami/Exonami-lotus/commit/a0109f6).

## 1.3 - 2026-10-07

- Add saved player names, a name-selection menu and an on-screen keyboard.
- Add win scores, winner selection on reset, a no-winner option and undo last win.
- Add three-player mode: one player across the top and two below.
- Add swipe-down ping and a four-second swipe-up undo, without a temporary hint.
- Add D6, D20 and coin flip on the middle hardware button; fix the dice-related restart.
- Add the last ten actions, opened by holding the centre icon.
- Add custom sleep artwork, rotated 90 degrees counterclockwise, and the Planeswalker menu icon.
- Remove life-screen hint text and arrows and the Settings image; fix the centre-circle redraw.
- Add manual and automatic screen cleanup, reduce unnecessary redraws and improve rendering and memory use.

[Source milestone](https://github.com/Ex0nami/Exonami-lotus/commit/16d9b59).

## 1.2 - 2026-09-15

- Initial custom build based on Sticky Lotus.
- Show cumulative signed life changes for four seconds after the last adjustment.
- Keep the upstream first-player roll.
- Fix Windows simulator builds, match device hold gestures and add build instructions and regression tests.

[Source milestone](https://github.com/Ex0nami/Exonami-lotus/commit/b7d35db).

## Release notes

Versions 1.2 and 1.3 group our earlier custom updates retrospectively. Version 1.3 includes the work from after the initial build through October 7; 1.3.1 contains the battery fix. Later versions include earlier fixes. The inherited v1.0.1, v1.1.0 and v1.1.1 tags belong to the original Sticky Lotus project.

Desktop regression tests passed. The feature build passed firmware checks and was installed with verified startup, power-latch initialization and saved-game restoration.

# Ping and uncluttered life screen

This update builds on the four-second-delta version confirmed working on the Sticky.

## Controls

On each player's life screen:

- Swipe **down from that player's reading direction**: every other player loses 1 life.
- Swipe up: open Poison. Swipe down inside Poison still only closes it.
- Swipe left or right: open Commander Damage, as before.

The swiping player's life is unchanged. Repeat pings accumulate the same signed
four-second life badge as normal life changes. Two-player mode affects just the
other player. Existing eliminated-player rules and the zero-life floor still apply.
Swipes starting on the central menu do not ping.

Only the POISON and COMMANDERDAMAGE hint text on the life screen was removed.
The small directional arrows and the counter screens remain. There is no new
on-screen button; swipe down is the ping action.

Swipe orientation now follows each player's rotated panel, including the whole
height of each panel in two-player mode. Previously that mode incorrectly used
the screen's upper/lower half to interpret swipes.

## Verification

The Windows simulator compiles. Shared-application tests passed for every seat
in two/four-player mode, both vertical halves of two-player panels, repeated
pings, badge expiry, poison editing and closing without pinging, Commander
navigation, the life floor, eliminated opponents, and menu-origin swipes.
The original four-second-delta and first-player-roll regression suite also passes.

Tests ran without a native graphics window, recording the real renderer's draw
calls. This update has not had a new graphical or physical-device test: session
permissions rejected native graphics execution and the firmware compiler launcher
fails with access denied in the sandbox. Elevated building was also rejected.
No new firmware was flashed; the previously working device firmware is unchanged.

To run the non-graphical tests:

```powershell
./custom/test-headless.ps1 -Compiler C:/tools/w64devkit/bin/g++.exe
```

The regular build script still builds the Windows simulator and graphical tests.
See custom/README.md for the original build setup. For a fresh firmware build,
use ESP-IDF 5.5.4 from the firmware directory:

```sh
idf.py build
idf.py merge-bin
```

Do not use the old firmware/release binary or the earlier custom ZIP for these
new features. They contain the previous version. Build this source first.

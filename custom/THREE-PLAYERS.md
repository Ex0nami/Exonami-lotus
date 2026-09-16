# Three-player mode

Settings now offers 2, 3 and 4 players. Three-player mode uses one full-width
800x240 top panel rotated toward the opposite seat and two 400x240 bottom panels.
The central menu remains at their junction. Life +/- hit areas follow each actual
panel, including the wide top panel. Multiplayer starting life applies to 3 and 4.

Existing ping, four-second same-player undo, life delta, poison, Commander damage,
first-player roll and Exonami sleep artwork are retained. Snapshots keep their
existing binary structure; the player-mode value 3 is added without changing it.
Changing player mode resets the game, matching the existing settings behavior.

Validation: desktop simulator build, headless and graphical integration suites
passed. Verified screenshots of the new layout and cumulative deltas. Tests cover
settings selection, full-width taps and holds, both lower seats, snapshot restore,
all seat orientations for ping/undo and poison/Commander navigation, timeout,
elimination, multiplayer life reset, and random first-player selection in 2/3/4.

Firmware build/install status is recorded separately below.

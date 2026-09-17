# History, Planeswalker menu and circle fix

On the game screen, hold the central on-screen symbol for at least 0.8 seconds
and release to open History. Tap (or a shorter hold) opens the existing Settings.
The physical middle button retains its dice/coin actions. Settings is unchanged.

The latest 10 actions are displayed newest first, five per page. Back returns to
the game; Previous/Next appear only when another page exists. Empty history has
an explicit No actions yet message. Touches in History never reach life controls.

Recorded: life, poison, committed Commander damage, ping, undo ping, dice/coin,
and first-player selection. Consecutive life changes for the same player within
four seconds of the previous change combine into one before/after entry. Another
recorded action or opening History ends that group. Ping and undo record the
actual affected players once, without duplicate life entries. No-op changes and
cancelled Commander edits are excluded. Zero-net life bursts remain visible as
an accurate record of adjustments made.

History is saved with the game when entering sleep in a separate versioned,
248-byte NVS blob. The existing game snapshot format is unchanged. Older firmware
saves without history open with an empty history. Invalid history is ignored.
Resetting the game, changing player mode, or changing the active starting-life
preset clears history because those actions start a new game. Abrupt power loss
can lose actions since the last sleep save, like the existing game state.

Includes the user-provided black Planeswalker icon and restoration of the centre
circle after regional updates. Source changes remain local on the custom branch.

Validation: headless and graphical application integration tests cover grouping,
4-second boundaries, retention, all action types, cancellation, safe restore,
centre gesture routing, pagination and reset. Both history pages were visually
checked. The prior pixel regression verifies the menu circle across all layouts.
Hardware persistence and gesture timing still require checking after installation.

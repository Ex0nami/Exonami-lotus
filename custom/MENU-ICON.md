# Planeswalker menu and redraw fix

The central lotus icon is replaced with the silhouette extracted from the
user-provided Planeswalker image. It is encoded as a black 32x44 one-bit bitmap;
the existing circular button and touch target remain unchanged.

Regional life updates now restore the menu and battery overlays after clearing
and drawing counters. This prevents overlapping player updates from erasing part
of the menu circle, including the full-width top panel in three-player mode.

Validation: a dedicated graphical regression test compares every pixel inside
the menu circle before and after each player update in 2/3/4-player modes. Passed.
The actual simulator screenshot was visually checked. Menu bitmap encoding is
round-trip verified; the build helper checks it is embedded in the firmware.

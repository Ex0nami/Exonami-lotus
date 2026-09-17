# Dice and coin

Middle hardware button: release a short press to roll, or hold about 0.8 seconds
to select Coin, D6 or D20. Release after a hold does not roll. Reset remains in
Settings; the bottom first-player button and power/sleep button are unchanged.
D6 is the default. The selected mode is saved alongside the game when sleeping,
using a separate NVS key so existing saved-game structure stays compatible.

Results appear for three seconds without animation. Tap to dismiss a result;
the dismissing tap never changes life. Selecting a mode closes the selector.
The previous screen is restored, including an in-progress Commander edit.
Opening dice cancels any pending ping undo. Life and other counters are unchanged.
Desktop controls: R rolls, D opens the selector, Escape closes it.

Tests: integration suite checks selection, invalid stored values, all outcomes
and ranges, expiry, input isolation, and existing game functionality. Simulator
screenshots are provided. Physical button timing and sleep persistence require
verification after installation.

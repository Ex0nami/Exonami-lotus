# Dice/coin restart fix

A live COM3 capture reproduced: "A stack overflow in task main has been detected."
The dice function in v1.1.1-15-gf3d26a5 reserves 2624 bytes (compiler DWARF frame
metadata) on a 3584-byte main task stack. Its local random_device seed temporary
keeps a large function frame alive during rendering. This can restart the device
and reload the last sleep-saved game, appearing to reset life totals.

The fix reuses the existing process-wide random generator already used for
first-player rolls. Dice no longer constructs a local random_device. It preserves
the same uniform Coin/D6/D20 outcomes and leaves controls and appearance unchanged.

The build helper now checks the actual ESP32 rollDice frame is no more than 512
bytes; the check rejects the previous binary. Integration tests start with life
33/28/17/9, poison and Commander damage, then verify repeated dice/coin rolls,
dismissal and timeout preserve them. All existing history/menu features remain.

Hardware retest required after installing the rebuilt firmware.

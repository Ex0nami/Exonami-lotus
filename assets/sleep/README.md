# Exonami portrait sleep artwork

Created with the built-in image-generation tool. The symbols are generated
recreations of white, blue, black, red and green mana, not downloaded vector assets.

Final refinement prompt:
"Refine this sleep-screen design without changing the word Exonami, its typography,
size, spacing, or portrait layout. Keep the white, blue, black, and green mana icons
and the pure white background. Correct only the FOURTH icon (red mana): it must be
the familiar Magic the Gathering RED MANA FIRE/FLAME symbol, a bold solid-black
flame silhouette with a rounded lower-left body and tapering jagged flame tongues
sweeping upward/right with white notches. It must NOT resemble a dragon, a bird,
or any animal; remove all head/eye/beak/claw-like details. The design is for a
480x800 monochrome e-paper screen: clean crisp black and white without gray shading
or paper texture. Exactly Exonami and the five mana symbols, no additional text."

Initial design: a centered large bold fantasy-serif Exonami wordmark at 40% of
the portrait height, with one horizontal row of five black mana silhouettes below
it at 55%, in W/U/B/R/G order; white background, generous margins, no extra text.

`exonami-source.png` is the generated source.
`exonami-portrait.png` is the final 480x800 one-bit portrait preview.
`exonami-panel.png` previews the current 800x480 panel framebuffer.
The panel artwork is rotated 90 degrees counterclockwise from the initial installed version. It is now upright in the normal landscape orientation. Only white margins are cropped; lettering and symbols retain their original size. Rotation is baked into the pixels because both existing Canvas backends
only support 0 and 180 degree bitmap rotations.

Recreate the framebuffer with `python custom/encode-sleep-art.py` (Pillow required).
The converter verifies that decoding and unrotating the bytes exactly recovers
the one-bit portrait preview. The runtime uses the same bitmap on desktop and
hardware. Hardware sleep now calls Application::showSleepScreen so it uses this same
renderer instead of the upstream hardware-only bitmap. Saved state,
power controls and wakeup handling remain unchanged. The sleep-only battery text
is omitted to keep the artwork clean; the live game battery display is unchanged.

Windows simulator compiled successfully. Firmware build/physical sleep display
are pending the user's local ESP-IDF build and subsequent installation.


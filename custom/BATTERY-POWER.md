# Battery power latch startup correction

The board component defines EnablePowerHold(), but app_main and run never called it. Startup now calls the existing board routine before NVS initialization, with error checking. The main component explicitly declares its board dependency. The call also runs on deep-sleep wake, which restarts app_main.

The existing routine drives HOLD (GPIO45) high and pulses LOCK (GPIO46) to set the external power latch. USB power can mask an unset latch. A latch retained from earlier firmware could explain prior successful battery use, but this explanation has not been confirmed on the device.

Evidence: the same device fails without USB on custom 1.3 even after reinstall and clearing NVS, while the user reports CrossInk 1.5.1 handles touch and repeated refreshes on battery. This is a candidate correction, not a hardware-verified fix. CrossInk's exact power implementation was not retrieved; no claim of code equivalence is made.

The change affects hardware startup only; desktop simulator tests cannot exercise the latch. Firmware compilation in the agent environment failed before compilation because Windows could not launch the toolchain (DLL relocation error). The separate battery-fix build helper builds and runs existing firmware checks, checks the linked latch routine and packages a merged image separately from the known v1.3 release.

Hardware acceptance: boot directly on battery, unplug USB during normal use, repeatedly update life, and test sleep/wake both on battery and across USB removal. Returning from CrossInk requires the complete custom image and matching partition layout, then restoring the saved NVS backup. Do not use the old application-only installer against CrossInk partitions.

## Validation update — 26 October 2026

The user-run build and firmware checks passed. Full-image installation verified the written firmware, logged power-latch initialization and restored the saved game. The user reports that battery operation appears to work again. Repeated sleep/wake, USB transitions and a longer game still need validation; this is an initial hardware success, not a completed endurance test.

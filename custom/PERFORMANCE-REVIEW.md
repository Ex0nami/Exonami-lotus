# Performance review — 20 September 2026

## Changes
- Black/white rectangle fills update packed bytes instead of calling setPixel for every pixel. Gray dithering keeps its existing path. Clipping and adjacent bits are preserved.
- ScreenManager avoids drawing a screen twice when draw() processes a navigation change.
- Once per minute, firmware logs internal free/minimum heap, largest free block, lifetime input-queue high-water mark, dropped gestures, and maximum scheduled refresh-service duration. Fixed counters only; queue snapshots use its existing mutex. Timing includes blocking panel service and retries, excludes immediate/manual refreshes. It is not average input latency.
- Includes the earlier manual Refresh screen button, bounded automatic cleanup, refresh batching/retry backoff, and history string formatting improvement.

## Wider audit
Reviewed application input/timeout paths, navigation, screen rendering, history storage/formatting, firmware loop and persistence, touch queue, canvas rasterization, panel framebuffer allocation and SPI transfers.
History is bounded to 10 records. Input uses a 32-entry ring. Panel framebuffers and DMA staging storage are allocated during initialization, not for each refresh. No growing collection was found in these paths; this does not prove absence of all leaks.
The driver compares unchanged frames and skips transmission. Actual partial refreshes still transfer both full framebuffers (96 KB) through a fixed staging buffer at 20 MHz. Its controller activation constraints make naive region-only writes unsafe. SPI speed and DMA chunk sizing were left unchanged pending hardware measurements.
The main loop drains at most four queued gestures per iteration. Panel service waits synchronously while the separate touch worker collects inputs. Busy waits yield CPU time, but the UI still waits for the panel. More CPU efficiency cannot eliminate physical e-paper refresh latency.
Existing dice stack fix remains. Stack usage of individual functions does not prove safety of every call chain. Build checks inspect the actual dice frame and report history formatter frame size.

## Validation and limits
20,000 deterministic randomized rectangle comparisons against a per-pixel reference check clipping, empty/reversed rectangles, both colors, untouched bits and guard bytes. Navigation regression verifies one draw per transition. Existing simulator tests cover game counters, histories, dice behavior, ping/undo, layouts and refresh scheduling.
No device speedup percentage is claimed. Firmware compilation and a long physical game are still required. Watch diagnostic trends for falling free/largest memory, growing queue high-water mark or dropped taps. Full cleanups should improve ghosting; physical results must be assessed on the panel.

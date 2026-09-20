# Screen cleanup and efficiency review

Settings now has Refresh screen above the branding. It forces a full panel
refresh and remains in Settings. Life, poison, Commander damage, history, dice
selection and player settings are not reset. A brief full-screen flash is normal.

Automatic refresh policy:
- Cleanup becomes due after 30 partial updates (previously 40).
- Prefer a three-second quiet period (previously ten seconds).
- Once cleanup is due, further updates cannot delay it more than 20 seconds.
- Nearby display changes batch over 35ms, bounded to 100ms under continuous input.
- Failed refreshes retry no faster than once per second instead of each 10ms loop.
- Successful manual/full refresh clears pending automatic cleanup.

Efficiency review:
- The old cleanup could be postponed indefinitely, including by badge expiry.
- Every update previously became immediately eligible for a blocking panel call.
- Repeated panel errors previously retried/logged on every service loop.
- History text construction used chained temporary strings. It now builds one
  reserved result using integer conversion without allocating numeric strings.
- History remains bounded to 10 records; touch input remains a bounded 32-frame
  queue. Those structures do not grow over a long game. This is not a heap-leak
  proof or a hardware performance measurement.
- The previous dice stack fix is retained and verified against the built ELF.

Validation: pure timing tests cover exact batching/idle/deadline boundaries,
continuous input, full-refresh cancellation and retry backoff. Application tests
verify manual refresh requests a full refresh without changing game/history.
Graphical integration tests passed and the new Settings screen was inspected.
Firmware build checks verify the refresh paths and report History formatter stack
usage from compiler metadata. Longer-game responsiveness and physical ghosting
still need verification on the device; e-ink refreshes remain blocking operations.

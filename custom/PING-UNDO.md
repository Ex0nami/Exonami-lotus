# Swipe-up ping undo

Swipe down in your player area to ping the other players. For four seconds,
swipe up in that same area to undo the latest ping. No hint or new control is drawn.
After the window expires, swipe up opens poison as before. Another gesture,
hardware button, screen change, reset, snapshot restore or sleep cancels undo.
Repeated pings replace the undo target: only the most recent ping is reversed.
Only life actually removed by that ping is restored, including a player brought
to zero life. Players already excluded by the existing life rules are unchanged.
Life-change badges use the existing cumulative delta behavior.

Validation: desktop simulator builds; integration tests exercise the actual
application/screens/renderer with a headless canvas, all seat orientations,
latest-ping-only reversal, four-second expiry, one-use undo, cancellation,
poison routing, and elimination. Physical verification follows firmware install.

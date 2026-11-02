# Changelog

## 1.4 — 2026-11-02

### What's new

- **Own commander damage:** record damage from your own commander, including when another player controls it.
- **Partner commanders:** toggle partners in the player-name menu. Track commanders A and B separately, each with its own 21-damage limit. Single commander is the default, with no extra prompt.
- **Move players between seats:** selecting an already-seated player moves them directly to the chosen seat. The previous seat becomes its numbered guest placeholder; players are not swapped. Wins stay with the profile; life totals and commander settings stay with the seat.
- **Battery startup fix:** enable the power latch on every startup, fixing the observed freeze when USB was disconnected.

### Saved games

Existing names, scores, history and game data migrate to the new save format. Existing commander damage becomes commander A's damage. Update without erasing NVS to retain saved data. Turning partners off is blocked while commander B has recorded damage; correct it to zero or reset the game first.

### Validation

Desktop regression tests passed. The feature build passed firmware checks and was installed with verified startup, power-latch initialization and saved-game restoration. 

#pragma once

#include "sticky_lotus/EliminationReason.h"
#include "sticky_lotus/GameSettings.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

namespace sticky_lotus {

/*
 * Maximale Spielerzahl der Anwendung.
 *
 * Wird sowohl von GameState als auch von GameSnapshot verwendet.
 */
inline constexpr std::size_t maximumPlayerCount = 4;

struct Player
{
    std::string name;
    int life = 40;
    int poison = 0;
    std::string deathMessage;

    EliminationReason lastEliminationReason =
        EliminationReason::None;
};

/*
 * Spielstand für Deep-Sleep.
 */
struct GameSnapshot
{
    static constexpr std::uint32_t validMagic =
        0x534C4F54; // "SLOT"

    std::uint32_t magic = 0;

    GameSettings settings{};

    std::array<
        int,
        maximumPlayerCount
    > life{};

    std::array<
        int,
        maximumPlayerCount
    > poison{};

    std::array<
        EliminationReason,
        maximumPlayerCount
    > eliminationReasons{};

    std::array<
        std::array<
            int,
            maximumPlayerCount
        >,
        maximumPlayerCount
    > commanderDamage{};
};

class GameState
{
public:
    static constexpr std::size_t maximumPlayerCount =
        sticky_lotus::maximumPlayerCount;

    GameState();

    [[nodiscard]]
    const Player& getPlayer(
        std::size_t index
    ) const;

    [[nodiscard]]
    std::size_t getPlayerCount() const;

    [[nodiscard]]
    const GameSettings& getSettings() const;

    [[nodiscard]]
    EliminationReason getEliminationReason(
        std::size_t playerIndex
    ) const;

    [[nodiscard]]
    bool isPlayerEliminated(
        std::size_t playerIndex
    ) const;

    void changeLife(
        std::size_t playerIndex,
        int amount
    );

    void setPlayerMode(
        PlayerMode mode
    );

    void setMultiplayerStartingLife(
        int life
    );

    void setTwoPlayerStartingLife(
        int life
    );

    void setSoundEnabled(
        bool enabled
    );

    void reset();

    void changeCommanderDamage(
        std::size_t attackerIndex,
        std::size_t defenderIndex,
        int amount
    );

    [[nodiscard]]
    int getCommanderDamage(
        std::size_t attackerIndex,
        std::size_t defenderIndex
    ) const;

    void changePoison(
        std::size_t playerIndex,
        int amount
    );

    [[nodiscard]]
    int getPoison(
        std::size_t playerIndex
    ) const;

    /*
     * Würfelt zufällig einen Startspieler unter den aktuell
     * aktiven Spielern aus und merkt ihn für die Hervorhebung
     * auf dem Game-Screen vor.
     *
     * Bewusst nicht Teil von GameSnapshot - reine, kurzlebige
     * UI-Rückmeldung, kein dauerhafter Spielzustand.
     */
    void rollStartingPlayer();

    [[nodiscard]]
    bool hasStartingPlayerHighlight() const;

    [[nodiscard]]
    std::size_t getStartingPlayerHighlight() const;

    void clearStartingPlayerHighlight();

    /*
     * Deep-Sleep-Unterstützung
     */
    [[nodiscard]]
    GameSnapshot createSnapshot() const;

    bool restoreSnapshot(
        const GameSnapshot& snapshot
    );

private:
    GameSettings settings_;

    std::array<
        Player,
        maximumPlayerCount
    > players_;

    bool hasStartingPlayerHighlight_ = false;
    std::size_t startingPlayerHighlight_ = 0;

    /*
     * Erster Index:
     * angreifender Commander
     *
     * Zweiter Index:
     * verteidigender Spieler
     */
    std::array<
        std::array<
            int,
            maximumPlayerCount
        >,
        maximumPlayerCount
    > commanderDamage_{};

    void resetCommanderDamage();

    void updateEliminationState(
        std::size_t playerIndex
    );
};

} // namespace sticky_lotus
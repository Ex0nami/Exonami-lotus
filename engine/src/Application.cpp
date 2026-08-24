#include "sticky_lotus/app/Application.h"

namespace sticky_lotus::app {

    Application::Application(
        ui::Canvas& canvas,
        input::InputProvider& input
    )
        : input_(input),
          renderer_(
              canvas
          ),
          screenManager_({
              game_,
              navigation_,
              commanderDraft_,
              renderer_
          })
    {
    }

    void Application::tick()
    {
        /*
         * Aktuelle Sound-Einstellung an die Plattform durchreichen.
         *
         * Der tatsächliche Klick entsteht asynchron im Touch-Task,
         * nicht in diesem Aufruf - regelmäßiges Synchronisieren
         * reicht, ein eigener Änderungs-Hook ist nicht nötig.
         */
        input_.setSoundEnabled(
            game_.getSettings().soundEnabled
        );

        const input::InputFrame input =
            input_.poll();

        const bool hasInput =
            input.gesture.gesture !=
                input::TouchGesture::None ||
            input.buttons.leftPressed ||
            input.buttons.centerPressed ||
            input.buttons.rightPressed ||
            input.cancelPressed;

        if (!hasInput) {
            return;
        }

        lastInputTime_ =
            std::chrono::steady_clock::now();

        screenManager_.handleInput(
            input
        );
    }

    void Application::checkTimeouts()
    {
        if (!game_.hasStartingPlayerHighlight()) {
            return;
        }

        /*
         * Nur relevant, während der Game-Screen tatsächlich
         * sichtbar ist - ein regionaler Redraw auf einem anderen
         * Screen würde dessen Inhalt beschädigen.
         */
        if (navigation_.currentScreen() != ScreenId::Game) {
            return;
        }

        constexpr std::chrono::seconds highlightTimeout{10};

        if (
            std::chrono::steady_clock::now() - lastInputTime_ <
            highlightTimeout
        ) {
            return;
        }

        const std::size_t highlightedPlayer =
            game_.getStartingPlayerHighlight();

        game_.clearStartingPlayerHighlight();

        renderer_.drawPlayerRegion(
            game_,
            highlightedPlayer
        );
    }

    void Application::rollStartingPlayer()
    {
        lastInputTime_ =
            std::chrono::steady_clock::now();

        game_.rollStartingPlayer();

        navigation_.showGame();

        /*
         * processScreenTransition() erkennt den Screenwechsel und
         * übernimmt onEnter/onExit sowie den vollständigen Redraw
         * inklusive Flush - derselbe Weg wie bei jeder anderen
         * Navigation.
         */
        screenManager_.draw();
    }

    void Application::resetGame()
    {
        lastInputTime_ =
            std::chrono::steady_clock::now();

        game_.reset();

        navigation_.showGame();

        screenManager_.draw();
    }

    void Application::draw()
    {
        screenManager_.draw();
    }

    void Application::setBatteryStatus(
        const int batteryPercent,
        const bool charging
    )
    {
        renderer_.setBatteryStatus(
            batteryPercent,
            charging
        );
    }
    void Application::showSleepScreen()
    {
        renderer_.drawSleepScreen();
    }
    GameSnapshot Application::createGameSnapshot() const
    {
        return game_.createSnapshot();
    }

    bool Application::restoreGameSnapshot(
        const GameSnapshot& snapshot
    )
    {
        const bool restored =
            game_.restoreSnapshot(
                snapshot
            );

        /*
         * Sofort synchronisieren statt auf den nächsten tick() zu
         * warten: tick() läuft erst, sobald bereits eine Eingabe
         * ansteht, aber der Klickton für genau diese erste Eingabe
         * entsteht asynchron im Touch-Task VOR diesem tick()-Aufruf.
         *
         * Ohne dieses sofortige Sync würde nach jedem Aufwachen aus
         * dem Deep Sleep der allererste Klick immer hörbar sein,
         * selbst wenn Sound eigentlich deaktiviert ist.
         */
        input_.setSoundEnabled(
            game_.getSettings().soundEnabled
        );

        return restored;
    }
} // namespace sticky_lotus::app
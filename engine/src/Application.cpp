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
        return game_.restoreSnapshot(
            snapshot
        );
    }
} // namespace sticky_lotus::app
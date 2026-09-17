#include "sticky_lotus/app/Application.h"
#include <random>

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
        // Clear expired labels before an input can navigate back to the game.
        const bool diceWasVisible = diceView_ != DiceView::Closed;
        checkTimeouts();
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
            input.cancelPressed || input.dicePressed || input.diceSelectorPressed;

        if (!hasInput) {
            return;
        }

        lastInputTime_ =
            std::chrono::steady_clock::now();

        if (input.diceSelectorPressed) { showDiceSelector(); return; }
        if (input.dicePressed) { rollDice(); return; }
        if (diceView_ == DiceView::History) {
            if (input.cancelPressed || input.buttons.centerPressed) { closeDice(); return; }
            if (input.gesture.gesture == input::TouchGesture::Tap) {
                const auto point = input.gesture.endPosition;
                if (ui::AppRenderer::historyButtonArea(1).contains(point)) { closeDice(); return; }
                if (historyPage_ > 0 && ui::AppRenderer::historyButtonArea(0).contains(point)) --historyPage_;
                else if (historyPage_ == 0 && game_.history().size() > 5 && ui::AppRenderer::historyButtonArea(2).contains(point)) ++historyPage_;
                draw();
            }
            return;
        }
        if (diceView_ != DiceView::Closed) {
            if (diceView_ == DiceView::Selector && input.gesture.gesture == input::TouchGesture::Tap) {
                const int values[]{2, 6, 20};
                for (int i = 0; i < 3; ++i) {
                    if (ui::AppRenderer::diceOptionArea(i).contains(input.gesture.endPosition)) {
                        setDiceSides(values[i]); closeDice(); return;
                    }
                }
            }
            if (input.cancelPressed || input.buttons.centerPressed || diceView_ == DiceView::Result)
                closeDice();
            return; // Never pass a modal tap through to life or counter controls.
        }
        if (diceWasVisible) return;
        if (navigation_.currentScreen() == ScreenId::Game &&
            input.gesture.gesture == input::TouchGesture::LongPress &&
            input.gesture.heldMilliseconds >= 800 &&
            renderer_.getMenuButtonRectangle().contains(input.gesture.startPosition) &&
            renderer_.getMenuButtonRectangle().contains(input.gesture.endPosition)) {
            showHistory(); return;
        }
        screenManager_.handleInput(
            input
        );
    }

    void Application::checkTimeouts()
    {
        if (diceView_ == DiceView::Result &&
            std::chrono::steady_clock::now() - diceShownAt_ >= std::chrono::seconds(3))
            closeDice();
        if (diceView_ != DiceView::Closed) return;

        /*
         * Jeder Zweig unten ist nur relevant, während der jeweils
         * betroffene Screen tatsächlich sichtbar ist - ein
         * regionaler Redraw auf einem anderen Screen würde dessen
         * Inhalt beschädigen.
         */
        const ScreenId currentScreen =
            navigation_.currentScreen();

        constexpr std::chrono::seconds deltaTimeout{15};

        const auto now =
            std::chrono::steady_clock::now();

        // Expire life bursts on every screen; only redraw a visible player.
        for (
            std::size_t playerIndex = 0;
            playerIndex < game_.getPlayerCount();
            ++playerIndex
        ) {
            if (!game_.hasLifeDeltaHighlight(playerIndex)) {
                continue;
            }

            if (
                now - game_.getLifeDeltaTimestamp(playerIndex) <
                GameState::lifeDeltaTimeout
            ) {
                continue;
            }

            game_.clearLifeDeltaHighlight(playerIndex);

            if (currentScreen == ScreenId::Game) {
                renderer_.drawPlayerRegion(game_, playerIndex);
            }
        }


        if (currentScreen == ScreenId::Game) {
            if (game_.hasStartingPlayerHighlight()) {
                constexpr std::chrono::seconds highlightTimeout{10};

                if (
                    now - lastInputTime_ >=
                    highlightTimeout
                ) {
                    const std::size_t highlightedPlayer =
                        game_.getStartingPlayerHighlight();

                    game_.clearStartingPlayerHighlight();

                    renderer_.drawPlayerRegion(
                        game_,
                        highlightedPlayer
                    );
                }
            }

            return;
        }

        if (currentScreen == ScreenId::Poison) {
            const std::size_t selectedPlayer =
                navigation_.selectedPlayer();

            if (
                selectedPlayer >= game_.getPlayerCount() ||
                !game_.hasPoisonDeltaHighlight(selectedPlayer)
            ) {
                return;
            }

            if (
                now - game_.getPoisonDeltaTimestamp(selectedPlayer) <
                deltaTimeout
            ) {
                return;
            }

            game_.clearPoisonDeltaHighlight(selectedPlayer);

            renderer_.drawPoisonRegion(
                game_,
                selectedPlayer
            );

            return;
        }

        if (currentScreen == ScreenId::CommanderDamage) {
            if (!commanderDraft_.isActive()) {
                return;
            }

            const std::size_t playerCount =
                commanderDraft_.playerCount();

            for (
                std::size_t attackingPlayer = 0;
                attackingPlayer < playerCount;
                ++attackingPlayer
            ) {
                if (
                    !commanderDraft_.hasDamageDeltaHighlight(
                        attackingPlayer
                    )
                ) {
                    continue;
                }

                if (
                    now -
                    commanderDraft_.getDamageDeltaTimestamp(
                        attackingPlayer
                    ) <
                    deltaTimeout
                ) {
                    continue;
                }

                commanderDraft_.clearDamageDeltaHighlight(
                    attackingPlayer
                );

                renderer_.drawCommanderDamageRegion(
                    game_,
                    commanderDraft_,
                    attackingPlayer
                );
            }
        }
    }

    void Application::rollStartingPlayer()
    {
        diceView_ = DiceView::Closed;
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
        diceView_ = DiceView::Closed;
        screenManager_.cancelPingUndo();
        lastInputTime_ =
            std::chrono::steady_clock::now();

        game_.reset();

        navigation_.showGame();

        screenManager_.draw();
    }

    void Application::draw()
    {
        if (diceView_ == DiceView::History) { renderer_.drawHistory(game_.history(), historyPage_); return; }
        screenManager_.draw();
        if (diceView_ != DiceView::Closed)
            renderer_.drawDice(diceView_ == DiceView::Selector, diceSides_, diceResult_);
    }

    void Application::showHistory()
    {
        screenManager_.cancelPingUndo();
        game_.history().finishBurst();
        lastInputTime_ = std::chrono::steady_clock::now();
        historyPage_ = 0;
        diceView_ = DiceView::History;
        draw();
    }

    void Application::setDiceSides(int sides)
    {
        if (sides == 2 || sides == 6 || sides == 20) diceSides_ = sides;
    }

    void Application::closeDice()
    {
        diceView_ = DiceView::Closed;
        screenManager_.draw();
    }

    void Application::showDiceSelector()
    {
        screenManager_.cancelPingUndo();
        lastInputTime_ = std::chrono::steady_clock::now();
        diceView_ = DiceView::Selector;
        draw();
    }

    void Application::rollDice()
    {
        static std::mt19937 rng{std::random_device{}()};
        screenManager_.cancelPingUndo();
        diceResult_ = std::uniform_int_distribution<int>(1, diceSides_)(rng);
        game_.history().add({HistoryKind::Dice, 0, diceSides_, 0, diceResult_});
        diceShownAt_ = lastInputTime_ = std::chrono::steady_clock::now();
        diceView_ = DiceView::Result;
        draw();
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
        diceView_ = DiceView::Closed;
        screenManager_.cancelPingUndo();
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
        diceView_ = DiceView::Closed;
        screenManager_.cancelPingUndo();
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

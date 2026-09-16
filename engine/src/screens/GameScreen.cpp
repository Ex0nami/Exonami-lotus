#include "sticky_lotus/screens/GameScreen.h"
#include "sticky_lotus/ui/AppRenderer.h"

#include <array>

namespace
{
    sticky_lotus::input::TouchGesture rotateGesture180(
        const sticky_lotus::input::TouchGesture gesture
    )
    {
        using sticky_lotus::input::TouchGesture;

        switch (gesture)
        {
        case TouchGesture::None:
            return TouchGesture::None;

        case TouchGesture::Tap:
            return TouchGesture::Tap;

        case TouchGesture::LongPress:
            return TouchGesture::LongPress;

        case TouchGesture::SwipeUp:
            return TouchGesture::SwipeDown;

        case TouchGesture::SwipeDown:
            return TouchGesture::SwipeUp;

        case TouchGesture::SwipeLeft:
            return TouchGesture::SwipeRight;

        case TouchGesture::SwipeRight:
            return TouchGesture::SwipeLeft;
        }

        return TouchGesture::None;
    }

} // namespace

namespace sticky_lotus::screens
{
    using input::TouchGesture;

    GameScreen::GameScreen(
        const ScreenContext context
    )
        : context_(context)
    {
    }

    void GameScreen::handleInput(
        const input::InputFrame& inputFrame
    )
    {
        if (inputFrame.buttons.centerPressed || inputFrame.buttons.leftPressed ||
            inputFrame.buttons.rightPressed)
            pingUndoActive_ = false;

        if (inputFrame.buttons.centerPressed)
        {
            context_.navigation.showSettings();
            return;
        }

        if (inputFrame.buttons.leftPressed)
        {
            context_.game.changeLife(0, -1);
        }

        if (inputFrame.buttons.rightPressed)
        {
            context_.game.changeLife(0, 1);
        }

        TouchGesture effectiveGesture =
            inputFrame.gesture.gesture;

        // Interpret swipes from the selected player's reading orientation.
        const bool isSwipe =
            effectiveGesture == TouchGesture::SwipeLeft ||
            effectiveGesture == TouchGesture::SwipeRight ||
            effectiveGesture == TouchGesture::SwipeUp ||
            effectiveGesture == TouchGesture::SwipeDown;

        if (
            isSwipe &&
            context_.renderer.isPlayerUpsideDown(
                getPlayerIndex(inputFrame.gesture.startPosition),
                context_.game.getPlayerCount()
            )
        )
        {
            effectiveGesture =
                rotateGesture180(
                    effectiveGesture
                );
        }

        if (effectiveGesture == TouchGesture::SwipeUp &&
            undoPing(inputFrame.gesture.startPosition))
            return;

        // Any other action consumes the opportunity; idle frames do not.
        if (effectiveGesture != TouchGesture::None)
            pingUndoActive_ = false;

        switch (effectiveGesture)
        {
        case TouchGesture::Tap:
            processTap(
                inputFrame.gesture.endPosition
            );
            break;
        case TouchGesture::LongPress:
            processLongPress(
                inputFrame.gesture.endPosition
            );
            break;
        case TouchGesture::SwipeLeft:
        case TouchGesture::SwipeRight:
            {
                input::GestureEvent effectiveEvent =
                    inputFrame.gesture;

                effectiveEvent.gesture =
                    effectiveGesture;

                processSwipe(
                    effectiveEvent
                );
                break;
            }

        case TouchGesture::SwipeUp:
            {
                const std::size_t playerIndex =
                    getPlayerIndex(
                        inputFrame.gesture.startPosition
                    );

                if (
                    playerIndex <
                    context_.game.getPlayerCount()
                )
                {
                    context_.navigation.showPoison(
                        playerIndex
                    );
                }

                break;
            }

        case TouchGesture::SwipeDown:
            processPing(inputFrame.gesture.startPosition);
            break;

        case TouchGesture::None:
            break;
        }
    }

    void GameScreen::processPing(const ui::Point position)
    {
        if (position.x < 0 || position.x >= ui::AppRenderer::screenWidth ||
            position.y < 0 || position.y >= ui::AppRenderer::screenHeight ||
            context_.renderer.getMenuButtonRectangle().contains(position))
        {
            return;
        }

        const std::size_t sourcePlayer = getPlayerIndex(position);
        const std::size_t playerCount = context_.game.getPlayerCount();
        std::array<bool, maximumPlayerCount> changed{};
        bool eliminationChanged = false;
        pingLoss_.fill(0);
        pingSource_ = sourcePlayer;
        pingPlayerCount_ = playerCount;

        for (std::size_t player = 0; player < playerCount; ++player)
        {
            if (player == sourcePlayer) continue;
            const int before = context_.game.getPlayer(player).life;
            const bool wasEliminated = context_.game.isPlayerEliminated(player);
            // Reuse life rules and the same cumulative four-second badge.
            context_.game.changeLife(player, -1);
            changed[player] = before != context_.game.getPlayer(player).life;
            pingLoss_[player] = before - context_.game.getPlayer(player).life;
            pingUndoActive_ |= changed[player];
            eliminationChanged |= wasEliminated != context_.game.isPlayerEliminated(player);
        }

        pingTime_ = std::chrono::steady_clock::now();
        if (eliminationChanged)
        {
            context_.renderer.drawGameImmediately(context_.game);
            return;
        }
        for (std::size_t player = 0; player < playerCount; ++player)
        {
            if (changed[player])
                context_.renderer.drawLifeCounterRegion(context_.game, player);
        }
    }

    bool GameScreen::undoPing(const ui::Point position)
    {
        if (!pingUndoActive_ ||
            std::chrono::steady_clock::now() - pingTime_ >= std::chrono::seconds(4) ||
            pingPlayerCount_ != context_.game.getPlayerCount() ||
            position.x < 0 || position.x >= ui::AppRenderer::screenWidth ||
            position.y < 0 || position.y >= ui::AppRenderer::screenHeight ||
            context_.renderer.getMenuButtonRectangle().contains(position) ||
            getPlayerIndex(position) != pingSource_)
            return false;

        pingUndoActive_ = false;
        bool eliminationChanged = false;
        for (std::size_t player = 0; player < pingPlayerCount_; ++player)
        {
            if (pingLoss_[player] == 0) continue;
            const bool wasEliminated = context_.game.isPlayerEliminated(player);
            context_.game.changeLife(player, pingLoss_[player]);
            eliminationChanged |= wasEliminated != context_.game.isPlayerEliminated(player);
        }
        if (eliminationChanged)
            context_.renderer.drawGameImmediately(context_.game);
        else
            for (std::size_t player = 0; player < pingPlayerCount_; ++player)
                if (pingLoss_[player] != 0)
                    context_.renderer.drawLifeCounterRegion(context_.game, player);
        return true;
    }

    void GameScreen::processTap(
        const ui::Point position
    )
    {
        if (
            context_.renderer
                    .getMenuButtonRectangle()
                    .contains(position)
        )
        {
            context_.navigation.showSettings();
            return;
        }

        const std::size_t playerIndex =
            getPlayerIndex(
                position
            );

        if (
            playerIndex >=
            context_.game.getPlayerCount()
        )
        {
            return;
        }

        int lifeChange =
            getLifeChange(
                position
            );

        const bool upsideDown =
            context_.renderer.isPlayerUpsideDown(
                playerIndex,
                context_.game.getPlayerCount()
            );

        if (upsideDown)
        {
            lifeChange =
                -lifeChange;
        }

        const int currentLife =
            context_.game.getPlayer(
                playerIndex
            ).life;

        if (
            currentLife == 0 &&
            lifeChange < 0
        )
        {
            return;
        }

        const bool wasEliminated =
    context_.game.isPlayerEliminated(
        playerIndex
    );

        const int lifeBefore =
            context_.game.getPlayer(
                playerIndex
            ).life;

        context_.game.changeLife(
            playerIndex,
            lifeChange
        );

        const int lifeAfter =
            context_.game.getPlayer(
                playerIndex
            ).life;

        const bool isEliminated =
            context_.game.isPlayerEliminated(
                playerIndex
            );

        /*
         * changeLife() hat die Eingabe verworfen.
         *
         * Das passiert z. B.:
         * - bei Life Loss auf 0
         * - bei Life-Änderungen nach Poison-Tod
         * - bei Life-Änderungen nach Commander-Tod
         *
         * Dann überhaupt nichts neu zeichnen.
         */
        if (lifeBefore == lifeAfter)
        {
            return;
        }

        /*
         * Wechsel lebend <-> ausgeschieden:
         * vollständiger Redraw und Full Refresh.
         */
        if (wasEliminated != isEliminated)
        {
            context_.renderer.drawGameImmediately(
                context_.game
            );

            return;
        }

        context_.renderer.drawLifeCounterRegion(
            context_.game,
            playerIndex
        );
    }

    void GameScreen::processLongPress(
        const ui::Point position
    )
    {
        const std::size_t playerIndex =
            getPlayerIndex(
                position
            );

        if (
            playerIndex >=
            context_.game.getPlayerCount()
        )
        {
            return;
        }

        int lifeChange =
            getLifeChange(
                position
            );

        const bool upsideDown =
            context_.renderer.isPlayerUpsideDown(
                playerIndex,
                context_.game.getPlayerCount()
            );

        if (upsideDown)
        {
            lifeChange =
                -lifeChange;
        }

        /*
         * Aus +/-1 wird +/-10.
         */
        lifeChange *= 10;
        const int currentLife =
            context_.game.getPlayer(
                playerIndex
            ).life;

        if (
            currentLife == 0 &&
            lifeChange < 0
        )
        {
            return;
        }


        const bool wasEliminated =
    context_.game.isPlayerEliminated(
        playerIndex
    );

        const int lifeBefore =
            context_.game.getPlayer(
                playerIndex
            ).life;

        context_.game.changeLife(
            playerIndex,
            lifeChange
        );

        const int lifeAfter =
            context_.game.getPlayer(
                playerIndex
            ).life;

        const bool isEliminated =
            context_.game.isPlayerEliminated(
                playerIndex
            );

        /*
         * changeLife() hat die Eingabe verworfen.
         *
         * Das passiert z. B.:
         * - bei Life Loss auf 0
         * - bei Life-Änderungen nach Poison-Tod
         * - bei Life-Änderungen nach Commander-Tod
         *
         * Dann überhaupt nichts neu zeichnen.
         */
        if (lifeBefore == lifeAfter)
        {
            return;
        }

        /*
         * Wechsel lebend <-> ausgeschieden:
         * vollständiger Redraw und Full Refresh.
         */
        if (wasEliminated != isEliminated)
        {
            context_.renderer.drawGameImmediately(
                context_.game
            );

            return;
        }

        context_.renderer.drawLifeCounterRegion(
            context_.game,
            playerIndex
        );
    }

    void GameScreen::processSwipe(
        const input::GestureEvent& gesture
    )
    {
        const std::size_t playerIndex =
            getPlayerIndex(
                gesture.startPosition
            );

        if (
            playerIndex >=
            context_.game.getPlayerCount()
        )
        {
            return;
        }

        /*
         * Der gewischte Spieler wird als ausgewählter Spieler gespeichert.
         *
         * Im nächsten Schritt erhält dieser Wert die klare Bedeutung:
         * Empfänger des Commander Damage.
         */
        context_.navigation.showCommanderDamage(
            playerIndex
        );
    }


    std::size_t GameScreen::getPlayerIndex(
        const ui::Point position
    ) const
    {
        for (std::size_t player = 0; player < context_.game.getPlayerCount(); ++player)
            if (context_.renderer.getPlayerRectangle(player, context_.game.getPlayerCount()).contains(position))
                return player;
        return context_.game.getPlayerCount();
    }

    int GameScreen::getLifeChange(const ui::Point position) const
    {
        const auto player = getPlayerIndex(position);
        if (player >= context_.game.getPlayerCount()) return 0;
        const auto area = context_.renderer.getPlayerRectangle(player, context_.game.getPlayerCount());
        return position.x < area.x + area.width / 2.0F ? -1 : 1;
    }

    void GameScreen::draw()
    {
        context_.renderer.drawGame(
            context_.game
        );
    }
} // namespace sticky_lotus::screens

#pragma once

#include "sticky_lotus/screens/Screen.h"
#include "sticky_lotus/screens/ScreenContext.h"

#include <cstddef>
#include <array>
#include <chrono>

namespace sticky_lotus::screens {

    /**
     * Normale Spielansicht mit Lebenspunkten und Menüknopf.
     */
    class GameScreen final : public Screen
    {
    public:
        explicit GameScreen(ScreenContext context);

        void handleInput(
            const input::InputFrame& inputFrame
        ) override;

        void draw() override;
        void cancelPingUndo() { pingUndoActive_ = false; }
        void onExit() override { cancelPingUndo(); }

    private:
        ScreenContext context_;
        bool pingUndoActive_ = false;
        std::size_t pingSource_ = 0;
        std::size_t pingPlayerCount_ = 0;
        std::array<int, maximumPlayerCount> pingLoss_{};
        std::chrono::steady_clock::time_point pingTime_{};
        bool undoPing(ui::Point position);

        void processTap(ui::Point position);
        void processPing(ui::Point position);
        void processSwipe(
            const input::GestureEvent& gesture
        );

        [[nodiscard]]
        std::size_t getPlayerIndex(
            ui::Point position
        ) const;

        [[nodiscard]]
        static int getLifeChange(
            ui::Point position
        );
        void processLongPress(
            ui::Point position
        );
    };

} // namespace sticky_lotus::screens

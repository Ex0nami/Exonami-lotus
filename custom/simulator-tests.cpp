// Integration tests use the real Application, screens, renderer and Raylib canvas.
#include "sticky_lotus/app/Application.h"
#ifndef STICKY_HEADLESS_TEST
#include "sticky_lotus_raylib/RaylibCanvas.h"
#include <rlgl.h>
#else
#include "HeadlessCanvas.h"
#endif
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>
using namespace sticky_lotus;
using namespace sticky_lotus::ui;
using namespace sticky_lotus::input;
using namespace std::chrono_literals;
void require(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
struct TestInput : InputProvider {
    InputFrame next{};
    InputFrame poll() override { auto result=next; next={}; return result; }
};
struct RecordedCanvas : Canvas {
#ifndef STICKY_HEADLESS_TEST
    sticky_lotus_raylib::RaylibCanvas real{800,480,"Sticky Lotus integration tests"};
#else
    HeadlessCanvas real;
#endif
    struct Label { std::string text; Rect area; int size; float rotation; };
    std::vector<Label> labels;
    int invalidations=0;
    int highlightedPlayers=0;
    void beginFrame() override { real.beginFrame(); }
    void endFrame() override { real.endFrame(); }
    void clear(Ink ink) override { real.clear(ink); }
    void fillRect(const Rect& area, Ink ink) override {
        if (ink==Ink::LightGray && area.width==400 && area.height>=240) ++highlightedPlayers;
        real.fillRect(area,ink);
    }
    void drawRect(const Rect& area, float thickness, Ink ink) override { real.drawRect(area,thickness,ink); }
    void fillRoundedRect(const Rect& area, float radius, Ink ink) override { real.fillRoundedRect(area,radius,ink); }
    void drawRoundedRect(const Rect& area, float radius, float thickness, Ink ink) override { real.drawRoundedRect(area,radius,thickness,ink); }
    void drawLine(Point start, Point end, float thickness, Ink ink) override { real.drawLine(start,end,thickness,ink); }
    void fillCircle(Point center, float radius, Ink ink) override { real.fillCircle(center,radius,ink); }
    void drawCircle(Point center, float radius, float thickness, Ink ink) override { real.drawCircle(center,radius,thickness,ink); }
    void flush() override { real.flush(); }
    void flushImmediately() override { real.flushImmediately(); }
    void drawMonochromeBitmap(const std::uint8_t* bitmap, int w, int h, int stride, const Rect& area, float rotation) override { real.drawMonochromeBitmap(bitmap,w,h,stride,area,rotation); }
    void invalidate(const Rect& area) override { ++invalidations; real.invalidate(area); }
    void drawText(std::string_view text, const Rect& area, int size, Ink ink, TextAlignment align, float rotation) override {
        labels.push_back({std::string(text),area,size,rotation});
        real.drawText(text,area,size,ink,align,rotation);
    }
    bool has(const std::string& text, int size=28, float rotation=-1) const {
        return std::any_of(labels.begin(),labels.end(),[&](const Label& l){return l.text==text && l.size==size && (rotation<0 || rotation==l.rotation);});
    }
};
int main() {
 try {
#ifndef STICKY_HEADLESS_TEST
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
#endif
    RecordedCanvas canvas;
#ifndef STICKY_HEADLESS_TEST
    SetTargetFPS(0); // Timing cases sleep explicitly; random trials need no frame delay.
#endif
    TestInput input;
    app::Application app(canvas,input);
    auto frame=[&](const char* screenshot=nullptr) {
        canvas.labels.clear(); canvas.highlightedPlayers=0; canvas.beginFrame(); app.draw();
        // Capture the current back buffer after flushing raylib's draw batch.
        if(screenshot) {
#ifndef STICKY_HEADLESS_TEST
            rlDrawRenderBatchActive();
            TakeScreenshot(screenshot);
#endif
        }
        canvas.endFrame();
    };
    auto tap=[&](float x,float y,TouchGesture gesture=TouchGesture::Tap) {
        input.next.gesture={gesture,{x,y},{x,y}}; app.tick(); app.checkTimeouts(); frame();
    };
    frame();
    const auto beforeDice=app.createGameSnapshot();
    require(app.diceSides()==6,"default die must be D6");
    app.setDiceSides(7); require(app.diceSides()==6,"invalid stored die must be ignored");
    input.next.diceSelectorPressed=true; app.tick(); frame("dice-selector.png");
    require(canvas.has("Choose a die or coin",28),"selector missing");
    tap(390,240); require(app.diceSides()==6,"D6 selection");
    for (int sides : {2,6,20}) {
        app.showDiceSelector();
        tap(sides==2 ? 200 : sides==6 ? 390 : 560,240);
        require(app.diceSides()==sides,"selector choice");
        std::vector<bool> seen(sides);
        for(int n=0;n<512;++n) {
            input.next.dicePressed=true; app.tick(); frame();
            int result=0;
            for(const auto& label:canvas.labels) if(label.size==80) {
                result=sides==2 ? (label.text=="HEADS" ? 1 : label.text=="TAILS" ? 2 : 0) : std::stoi(label.text);
            }
            require(result>=1 && result<=sides,"dice result out of range"); seen[result-1]=true;
        }
        require(std::all_of(seen.begin(),seen.end(),[](bool v){return v;}),"dice outcome unreachable");
    }
    frame("dice-result.png");
    tap(50,360); // Dismiss only, never subtract life.
    require(app.createGameSnapshot().life==beforeDice.life,"dice modal changed life");
    app.rollDice(); std::this_thread::sleep_for(3100ms); app.checkTimeouts(); frame();
    require(!canvas.has("D20",28),"dice result did not expire");
    app.showDiceSelector(); input.next.cancelPressed=true; app.tick(); frame();
    require(!canvas.has("Choose a die or coin",28),"selector cancel");
    require(app.createGameSnapshot().poison==beforeDice.poison && app.createGameSnapshot().commanderDamage==beforeDice.commanderDamage,"dice altered counters");
    // Core history: bounded storage, burst boundaries and safe restore.
    ActionHistory log;
    const auto start=ActionHistory::Clock::now();
    log.add({HistoryKind::Life,0,0,40,39},true,start);
    log.add({HistoryKind::Life,0,0,39,33},true,start+3999ms);
    require(log.size()==1 && log.text(0)=="P1 Life: 40 -> 33 (-7)","life burst history");
    log.add({HistoryKind::Life,0,0,33,35},true,start+4000ms);
    require(log.size()==1 && log.entry(0).after==35,"burst timer must extend with last change");
    log.add({HistoryKind::Life,0,0,35,34},true,start+8000ms);
    require(log.size()==2,"exact four-second history boundary");
    for(int n=1;n<=12;++n) log.add({HistoryKind::Dice,0,20,0,n});
    require(log.size()==10 && log.entry(0).after==12 && log.entry(9).after==3,"latest ten, newest first");
    ActionHistory restoredHistory;
    require(restoredHistory.restore(log.snapshot()) && restoredHistory.text(9)=="D20: 3","history persistence roundtrip");
    auto invalidHistory=log.snapshot(); invalidHistory.count=11;
    require(!restoredHistory.restore(invalidHistory) && restoredHistory.size()==0,"reject oversized history");
    invalidHistory=log.snapshot(); invalidHistory.entries[0].kind=static_cast<HistoryKind>(99);
    require(!restoredHistory.restore(invalidHistory),"reject unknown history kind");
    sticky_lotus::GameState historyGame;
    historyGame.changePoison(0,-1); require(historyGame.history().size()==0,"ignore no-op counter changes");
    historyGame.changeLife(0,-7); historyGame.changeLife(0,2);
    require(historyGame.history().text(0)=="P1 Life: 40 -> 35 (-5)","GameState cumulative life history");
    historyGame.changePoison(1,1);
    commander::CommanderDamageDraft historyDraft;
    historyDraft.begin(historyGame,1); historyDraft.changeDamage(0,3); historyDraft.cancel();
    require(historyGame.history().size()==2,"cancelled Commander draft must not log");
    historyDraft.begin(historyGame,1); historyDraft.changeDamage(0,3); historyDraft.commit(historyGame);
    require(historyGame.history().size()==3 && historyGame.history().text(0)=="P1 -> P2 Commander: 0 -> 3","committed Commander action without duplicate life entry");
    historyGame.reset(); require(historyGame.history().size()==0,"reset clears history");
    historyGame.rollStartingPlayer(); require(historyGame.history().entry(0).kind==HistoryKind::FirstPlayer,"first-player history");
    historyGame.setPlayerMode(PlayerMode::ThreePlayers); require(historyGame.history().size()==0,"player mode clears history");
    // Actual app routing: centre hold must never alter a player's life.
    app.resetGame(); frame();
    app.showHistory(); frame("history-empty.png"); require(canvas.has("No actions yet",28),"empty history screen");
    tap(400,430);
    tap(400,240,TouchGesture::LongPress); // Legacy/short hold opens settings safely.
    require(app.createGameSnapshot().life[3]==40 && canvas.has("Roll for first",24),"short centre hold must open settings, not change life");
    tap(645,411);
    tap(50,360); tap(50,360); tap(350,360);
    require(app.createHistorySnapshot().count==1 && app.createHistorySnapshot().entries[0].after==39,"app life history grouping");
    tap(200,100,TouchGesture::SwipeUp); // Ping from rotated P1.
    require(app.createHistorySnapshot().count==2 && app.createHistorySnapshot().entries[0].kind==HistoryKind::Ping && app.createHistorySnapshot().entries[0].affected==14,"ping must be one entry with exact targets");
    tap(200,100,TouchGesture::SwipeDown);
    require(app.createHistorySnapshot().count==3 && app.createHistorySnapshot().entries[0].kind==HistoryKind::UndoPing,"undo ping must be one entry");
    const auto savedLife=app.createGameSnapshot(); const auto savedHistory=app.createHistorySnapshot();
    app.resetGame(); require(app.restoreGameSnapshot(savedLife) && app.restoreHistorySnapshot(savedHistory),"restore game and history together");
    input.next.gesture={TouchGesture::LongPress,{400,240},{400,240},800}; app.tick(); frame();
    require(canvas.has("History",36) && app.createGameSnapshot().life==savedLife.life,"800ms centre hold history routing");
    tap(50,360); require(app.createGameSnapshot().life==savedLife.life,"history consumes underlying life taps");
    tap(400,430);
    require(app.restoreHistorySnapshot(log.snapshot()),"ten entry setup");
    app.showHistory(); frame("history-page1.png");
    require(canvas.has("D20: 12",24) && canvas.has("D20: 8",24) && !canvas.has("D20: 7",24),"first page newest five");
    tap(650,430); frame("history-page2.png");
    require(canvas.has("D20: 7",24) && canvas.has("D20: 3",24) && !canvas.has("D20: 8",24),"second page older five");
    tap(150,430); require(canvas.has("D20: 12",24),"history previous button");
    ActionHistory preview;
    preview.add({HistoryKind::FirstPlayer,0});
    preview.add({HistoryKind::Dice,0,2,0,2});
    preview.add({HistoryKind::Life,0,0,40,33});
    preview.add({HistoryKind::Poison,1,0,0,1});
    preview.add({HistoryKind::Commander,1,2,0,4});
    preview.add({HistoryKind::Ping,0,0,0,0,14});
    preview.add({HistoryKind::UndoPing,0,0,0,0,14});
    preview.add({HistoryKind::Dice,0,20,0,17});
    preview.add({HistoryKind::Life,3,0,40,37});
    preview.add({HistoryKind::Dice,0,6,0,4});
    require(app.restoreHistorySnapshot(preview.snapshot()),"mixed history preview restore");
    app.showHistory(); frame("history-preview-page1.png");
    tap(650,430); frame("history-preview-page2.png");
    tap(400,430); app.resetGame(); frame();
    std::cout << "PASS: history bursts, bounded retention, ping/undo, committed counters, persistence, reset, centre hold and pagination.\n";
    for(int i=0;i<7;++i) tap(50,360);
    require(app.createGameSnapshot().life[2]==33,"seven taps: expected 33 life");
    require(canvas.has("-7",28,0),"missing cumulative -7");
    frame("four-players-minus7.png");
    tap(350,360); tap(350,360);
    require(app.createGameSnapshot().life[2]==35 && canvas.has("-5"),"mixed signs must net -5");
    tap(350,120); // Upper player is rotated: right-hand screen side loses life.
    require(app.createGameSnapshot().life[0]==39 && canvas.has("-1",28,180),"rotated player input/label");
    frame("four-players-mixed.png");
    tap(50,360,TouchGesture::LongPress);
    require(app.createGameSnapshot().life[2]==25 && canvas.has("-15"),"long press must accumulate -10");
    std::this_thread::sleep_for(2200ms);
    tap(350,360);
    std::this_thread::sleep_for(1950ms); app.checkTimeouts(); frame();
    require(canvas.has("-14") && !canvas.has("-1"),"per-player independent expiry / timer extension");
    std::this_thread::sleep_for(2150ms);
    canvas.invalidations=0; app.checkTimeouts();
    require(canvas.invalidations>0,"idle expiry must request refresh");
    frame("expired.png"); require(!canvas.has("-14"),"idle expiry must clear badge");
    tap(350,360); require(canvas.has("+1"),"new burst after expiry");
    tap(50,360); require(!canvas.has("+1") && !canvas.has("-1"),"zero net hides label");
    tap(50,360); require(canvas.has("-1"),"change after zero net");
    tap(400,240); require(canvas.has("Roll for first",24),"settings roll button missing");
    std::this_thread::sleep_for(4100ms);
    canvas.invalidations=0; app.checkTimeouts();
    require(canvas.invalidations==0,"expiry must not redraw game over settings");
    tap(645,411); require(!canvas.has("-1"),"expired delta survived navigation");
    tap(400,240); tap(400,411); // Exercise actual settings button.
    require(canvas.has("40",90),"roll did not return to game");
    require(canvas.highlightedPlayers==1,"roll button must visibly highlight exactly one player");
    frame("roll-for-first.png");
    // Two-player render/input and unchanged reset/snapshot semantics.
    auto snap=app.createGameSnapshot(); snap.settings.playerMode=PlayerMode::TwoPlayers;
    snap.life.fill(20); require(app.restoreGameSnapshot(snap),"restore two-player snapshot");
    app.resetGame(); frame();
    tap(350,240); tap(750,240);
    require(app.createGameSnapshot().life[0]==19 && app.createGameSnapshot().life[1]==21,"two-player tap mapping");
    require(canvas.has("-1",28,180) && canvas.has("+1",28,0),"two-player signed/rotated badges");
    frame("two-players.png");
    app.resetGame(); frame(); require(!canvas.has("-1") && !canvas.has("+1"),"reset must clear badges");
    // Select the new mode through the actual settings control.
    tap(400,240); tap(202,147); tap(645,411);
    require(app.createGameSnapshot().settings.playerMode==PlayerMode::ThreePlayers,"three-player settings button");
    frame("three-players.png");
    require(app.createGameSnapshot().life[0]==40,"three-player multiplayer starting life");
    tap(300,100); tap(500,100); tap(700,100,TouchGesture::LongPress);
    require(app.createGameSnapshot().life[0]==30,"full-width top panel must split at screen center");
    tap(50,360); tap(750,360);
    require(app.createGameSnapshot().life[1]==39 && app.createGameSnapshot().life[2]==41 && app.createGameSnapshot().life[3]==40,"bottom life touch areas and inactive fourth seat");
    frame("three-players-changes.png");
    const auto threeSaved=app.createGameSnapshot(); app.resetGame();
    require(app.restoreGameSnapshot(threeSaved) && app.createGameSnapshot().life==threeSaved.life && app.createGameSnapshot().settings.playerMode==PlayerMode::ThreePlayers,"three-player snapshot roundtrip");
    // Test ping from every seat and both halves of each two-player panel.
    for (auto mode : {PlayerMode::FourPlayers, PlayerMode::ThreePlayers, PlayerMode::TwoPlayers}) {
        for (std::size_t seat=0; seat<static_cast<std::size_t>(mode); ++seat) {
            for (float localY : {60.0F,180.0F}) {
                app.resetGame(); // Clear transient deltas between independent cases.
                auto initial=app.createGameSnapshot();
                initial.settings.playerMode=mode; initial.life.fill(40);
                initial.poison.fill(0); for(auto& row:initial.commanderDamage) row.fill(0);
                require(app.restoreGameSnapshot(initial),"ping restore");
                // Restore sets starting totals; reset above clears transient labels.
                const bool rotated=mode!=PlayerMode::FourPlayers ? seat==0 : seat<2;
                const float x=mode==PlayerMode::ThreePlayers ? (seat==0 ? 400.0F : (seat-1)*400.0F+200.0F) : (seat%2)*400.0F+200.0F;
                const float y=mode==PlayerMode::TwoPlayers ? (localY==60 ? 100.0F : 380.0F) : (mode==PlayerMode::ThreePlayers ? (seat==0 ? 0.0F : 240.0F) : (seat/2)*240.0F)+localY;
                const auto down=rotated ? TouchGesture::SwipeUp : TouchGesture::SwipeDown;
                tap(x,y,down);
                const auto result=app.createGameSnapshot();
                for(std::size_t p=0;p<static_cast<std::size_t>(mode);++p)
                    require(result.life[p]==(p==seat ? 40 : 39),"ping must affect every other active seat exactly once");
                require(!canvas.has("POISON",14) && !canvas.has("COMMANDERDAMAGE",14),"game hint text still present");
                tap(x,y,down);
                require(canvas.has("-2"),"repeat ping must accumulate delta");
                tap(x,y,rotated ? TouchGesture::SwipeDown : TouchGesture::SwipeUp);
                for(std::size_t p=0;p<static_cast<std::size_t>(mode);++p)
                    require(app.createGameSnapshot().life[p]==(p==seat ? 40 : 39),"undo must reverse only latest ping from every orientation");
                require(canvas.has("-1"),"undo must subtract from cumulative delta");
                tap(x,y,down);
                if(mode==PlayerMode::FourPlayers && seat==0 && localY==60)
                    frame("ping-player1.png");
            }
        }
    }
    std::this_thread::sleep_for(4100ms); app.checkTimeouts(); frame();
    require(!canvas.has("-2"),"ping delta must expire after four seconds");
    for(int action=0;action<3;++action) {
        app.resetGame();
        auto initial=app.createGameSnapshot(); initial.settings.playerMode=PlayerMode::FourPlayers;
        initial.life.fill(40); require(app.restoreGameSnapshot(initial),"undo lifecycle setup");
        tap(200,100,TouchGesture::SwipeUp);
        if(action==0) app.resetGame();
        if(action==1) require(app.restoreGameSnapshot(app.createGameSnapshot()),"undo lifecycle restore");
        if(action==2) app.showSleepScreen();
        const auto lifeBefore=app.createGameSnapshot().life;
        tap(200,100,TouchGesture::SwipeDown);
        require(app.createGameSnapshot().life==lifeBefore,"reset, restore and sleep must cancel undo");
    }
    // Screen-level routing keeps poison and Commander functions intact.
    sticky_lotus::GameState routedGame;
    app::NavigationState navigation;
    commander::CommanderDamageDraft draft;
    AppRenderer renderer(canvas);
    screens::ScreenManager screens({routedGame,navigation,draft,renderer});
    auto gesture=[&](float x,float y,TouchGesture g) {
        InputFrame f{}; f.gesture={g,{x,y},{x,y}}; screens.handleInput(f);
    };
    for(auto mode : {PlayerMode::FourPlayers,PlayerMode::ThreePlayers,PlayerMode::TwoPlayers}) {
        routedGame.setPlayerMode(mode);
        for(std::size_t seat=0;seat<routedGame.getPlayerCount();++seat) {
            const bool rotated=renderer.isPlayerUpsideDown(seat,routedGame.getPlayerCount());
            const auto area=renderer.getPlayerRectangle(seat,routedGame.getPlayerCount());
            const float x=area.x+150, y=area.y+area.height*0.75F;
            gesture(x,y,rotated ? TouchGesture::SwipeDown : TouchGesture::SwipeUp);
            require(navigation.currentScreen()==app::ScreenId::Poison && navigation.selectedPlayer()==seat,"poison navigation broken");
            gesture(x,area.y+area.height*(rotated ? 0.75F : 0.25F),TouchGesture::Tap);
            require(routedGame.getPlayer(seat).poison==1,"poison editing broken");
            auto before=routedGame.createSnapshot().life;
            gesture(x,y,rotated ? TouchGesture::SwipeUp : TouchGesture::SwipeDown);
            require(navigation.currentScreen()==app::ScreenId::Game && routedGame.createSnapshot().life==before,"closing poison must not ping");
            for(auto sideways : {TouchGesture::SwipeLeft,TouchGesture::SwipeRight}) {
                gesture(x,y,sideways);
                require(navigation.currentScreen()==app::ScreenId::CommanderDamage && draft.isActive(),"Commander navigation broken");
                gesture(x,y,sideways);
                require(navigation.currentScreen()==app::ScreenId::Game && !draft.isActive(),"Commander exit broken");
            }
        }
    }
    routedGame.setPlayerMode(PlayerMode::FourPlayers);
    routedGame.changeLife(1,-39); routedGame.changePoison(2,10);
    gesture(200,100,TouchGesture::SwipeUp); // Player 1's downward swipe.
    require(routedGame.getPlayer(0).life==40 && routedGame.getPlayer(1).life==0 &&
            routedGame.getPlayer(2).life==40 && routedGame.getPlayer(3).life==39,
            "ping must preserve elimination rules and life floor");
    auto before=routedGame.createSnapshot().life;
    gesture(200,100,TouchGesture::SwipeDown);
    require(routedGame.getPlayer(1).life==1 && routedGame.getPlayer(2).life==40 &&
            routedGame.getPlayer(3).life==40 && navigation.currentScreen()==app::ScreenId::Game,
            "undo must revive life elimination and leave poison-eliminated player alone");
    gesture(200,100,TouchGesture::SwipeDown);
    require(navigation.currentScreen()==app::ScreenId::Poison,"undo must be single-use");
    gesture(200,100,TouchGesture::SwipeUp); // Close poison.
    gesture(200,100,TouchGesture::SwipeUp); // Ping.
    std::this_thread::sleep_for(4100ms);
    gesture(200,100,TouchGesture::SwipeDown);
    require(navigation.currentScreen()==app::ScreenId::Poison,"expired undo must open poison");
    gesture(200,100,TouchGesture::SwipeUp);
    gesture(200,100,TouchGesture::SwipeUp);
    gesture(600,360,TouchGesture::SwipeUp);
    require(navigation.currentScreen()==app::ScreenId::Poison && navigation.selectedPlayer()==3,"other player swipe must open poison");
    gesture(600,360,TouchGesture::SwipeDown);
    gesture(200,100,TouchGesture::SwipeDown);
    require(navigation.currentScreen()==app::ScreenId::Poison,"navigation must cancel pending undo");
    gesture(200,100,TouchGesture::SwipeUp);
    gesture(200,100,TouchGesture::SwipeUp);
    gesture(50,100,TouchGesture::Tap);
    gesture(200,100,TouchGesture::SwipeDown);
    require(navigation.currentScreen()==app::ScreenId::Poison,"life tap must cancel undo");
    gesture(200,100,TouchGesture::SwipeUp);
    before=routedGame.createSnapshot().life;
    gesture(400,240,TouchGesture::SwipeDown);
    require(routedGame.createSnapshot().life==before,"menu-origin swipe must not ping");
    std::cout << "PASS: ping all seats, player-relative directions, cumulative expiry, hidden hint text, poison edit/close, Commander navigation, elimination and menu guard.\n";
    sticky_lotus::GameState game;
    game.setPlayerMode(PlayerMode::ThreePlayers); game.setMultiplayerStartingLife(30);
    require(game.getPlayerCount()==3 && game.getPlayer(0).life==30 && game.getPlayer(2).life==30,"three-player starting life setting must reset game");
    for(auto mode : {PlayerMode::TwoPlayers,PlayerMode::ThreePlayers,PlayerMode::FourPlayers}) {
        game.setPlayerMode(mode); std::vector<bool> seen(game.getPlayerCount());
        for(int n=0;n<512;++n) { game.rollStartingPlayer(); require(game.hasStartingPlayerHighlight(),"roll disabled"); auto p=game.getStartingPlayerHighlight(); require(p<game.getPlayerCount(),"roll outside active players"); seen[p]=true; }
        require(std::all_of(seen.begin(),seen.end(),[](bool v){return v;}),"roll never selected one of active players");
    }
    game.changeLife(0,-7); std::this_thread::sleep_for(4100ms); game.changeLife(0,2);
    require(game.getLifeDelta(0)==2,"stale burst accumulated before UI timeout service");
    game.changePoison(0,10); auto life=game.getPlayer(0).life; game.changeLife(0,10);
    require(game.getPlayer(0).life==life,"poison elimination behavior changed");
    game.reset(); game.changeLife(0,-100); require(game.getPlayer(0).life==0 && !game.hasLifeDeltaHighlight(0),"life floor/elimination behavior changed");
    game.changeLife(0,1); require(game.getLifeDelta(0)==1,"revival must begin new burst");
    std::cout << "PASS: cumulative taps, mixed signs, holds, rotations, independent timers, idle expiry, navigation, zero net, reset, snapshots, elimination, and 2/4-player roll.\n";
    return 0;
 } catch(const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}

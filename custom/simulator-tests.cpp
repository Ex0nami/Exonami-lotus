// Integration tests use the real Application, screens, renderer and Raylib canvas.
#include "sticky_lotus/app/Application.h"
#include "sticky_lotus_raylib/RaylibCanvas.h"
#include <rlgl.h>
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
    sticky_lotus_raylib::RaylibCanvas real{800,480,"Sticky Lotus integration tests"};
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
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    RecordedCanvas canvas;
    TestInput input;
    app::Application app(canvas,input);
    auto frame=[&](const char* screenshot=nullptr) {
        canvas.labels.clear(); canvas.highlightedPlayers=0; canvas.beginFrame(); app.draw();
        // Capture the current back buffer after flushing raylib's draw batch.
        if(screenshot) {
            rlDrawRenderBatchActive();
            TakeScreenshot(screenshot);
        }
        canvas.endFrame();
    };
    auto tap=[&](float x,float y,TouchGesture gesture=TouchGesture::Tap) {
        input.next.gesture={gesture,{x,y},{x,y}}; app.tick(); app.checkTimeouts(); frame();
    };
    frame();
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
    sticky_lotus::GameState game;
    for(auto mode : {PlayerMode::TwoPlayers,PlayerMode::FourPlayers}) {
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

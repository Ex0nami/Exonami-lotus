#include "sticky_lotus/GameState.h"
#include "sticky_lotus/ui/AppRenderer.h"
#include "sticky_lotus_raylib/RaylibCanvas.h"
#include <raylib.h>
#include <rlgl.h>
#include <stdexcept>
#include <iostream>
int main() {
 try {
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    sticky_lotus_raylib::RaylibCanvas canvas(800,480,"Menu redraw regression");
    sticky_lotus::ui::AppRenderer renderer(canvas);
    sticky_lotus::GameState game;
    for(auto mode : {PlayerMode::TwoPlayers,PlayerMode::ThreePlayers,PlayerMode::FourPlayers}) {
        game.setPlayerMode(mode);
        canvas.beginFrame(); renderer.drawGame(game); rlDrawRenderBatchActive();
        Image original=LoadImageFromScreen();
        for(std::size_t player=0;player<game.getPlayerCount();++player) {
            game.changeLife(player,-1); renderer.drawLifeCounterRegion(game,player);
            rlDrawRenderBatchActive(); Image changed=LoadImageFromScreen();
            for(int y=210;y<=270;++y) for(int x=370;x<=430;++x) {
                if((x-400)*(x-400)+(y-240)*(y-240)>30*30) continue;
                auto a=GetImageColor(original,x,y),b=GetImageColor(changed,x,y);
                if(a.r!=b.r || a.g!=b.g || a.b!=b.b) throw std::runtime_error("Menu pixels changed after regional life redraw");
            }
            UnloadImage(changed);
        }
        if(mode==PlayerMode::FourPlayers) TakeScreenshot("menu-after-updates.png");
        UnloadImage(original); canvas.endFrame();
    }
    std::cout << "PASS: full menu circle and icon unchanged after every player update in 2/3/4-player layouts.\n";
 } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}

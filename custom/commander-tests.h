#pragma once
void commanderTests(RecordedCanvas& canvas) {
    sticky_lotus::GameState g;
    require(!g.hasPartners(0),"single commander default");
    g.changeCommanderDamage(0,0,7);
    require(g.getCommanderDamage(0,0)==7 && g.getPlayer(0).life==33,"own commander changes life");
    g.changeCommanderDamage(0,0,14);
    require(g.getEliminationReason(0)==EliminationReason::CommanderDamage,"own commander lethal");
    g.changeCommanderDamage(0,0,-1);
    require(!g.isPlayerEliminated(0),"own commander correction revives");
    g.reset(); require(g.setPartners(1,true),"enable partners");
    g.changeCommanderDamage(1,0,11,0);g.changeCommanderDamage(1,0,10,1);
    require(!g.isPlayerEliminated(0) && g.getPlayer(0).life==19,"A+B do not combine to 21");
    require(!g.setPartners(1,false),"cannot hide B damage");
    g.changeCommanderDamage(1,0,11,1);
    require(g.getEliminationReason(0)==EliminationReason::CommanderDamage,"B independently lethal");
    sticky_lotus::GameState restored; require(restored.restoreSnapshot(g.createSnapshot()),"new snapshot restores");
    require(restored.hasPartners(1) && restored.getCommanderDamage(1,0,1)==21 && restored.isPlayerEliminated(0),"partner snapshot data and elimination");
    ActionHistory h;require(h.restore(g.history().snapshot()),"partner history restores");
    require(h.text(0).find("P2 B")!=std::string::npos,"history identifies B");
    auto bad=g.createSnapshot();bad.partners[1]=0;
    require(!restored.restoreSnapshot(bad),"reject hidden B damage in saved state");
    GameSnapshotV1 old{};old.magic=GameSnapshotV1::validMagic;old.life.fill(40);old.commanderDamage[2][1]=9;
    GameSnapshot migrated{};static_cast<GameSnapshotV1&>(migrated)=old;
    require(restored.restoreSnapshot(migrated) && !restored.hasPartners(2) && restored.getCommanderDamage(2,1)==9,"v1 migration keeps A and defaults single");
    g.reset();require(g.hasPartners(1) && g.getCommanderDamage(1,0,1)==0,"reset retains configuration but clears B");
    require(g.setPartners(1,false),"disable after reset");
    commander::CommanderDamageDraft draft;
    g.setPartners(0,true);draft.begin(g,0);draft.changeDamage(0,4);draft.selectCommander(0,1);draft.changeDamage(0,6);
    require(draft.damageFrom(0)==6 && !draft.isLethal(),"independent own partner draft");
    draft.commit(g);require(g.getCommanderDamage(0,0)==4 && g.getCommanderDamage(0,0,1)==6 && g.getPlayer(0).life==30,"commit both once");
    draft.commit(g);require(g.getPlayer(0).life==30,"no duplicate commit");
    draft.begin(g,0);draft.selectCommander(0,1);draft.changeDamage(0,15);require(draft.isLethal(),"draft B lethal");draft.cancel();
    require(g.getCommanderDamage(0,0,1)==6,"cancel preserves B");
    TestInput input;app::Application application(canvas,input);
    auto tap=[&](Rect area){Point p{area.x+area.width/2,area.y+area.height/2};input.next.gesture={TouchGesture::Tap,p,p};application.tick();};
    tap(AppRenderer::playerNameArea(0,4));tap(AppRenderer::partnerToggle());
    require(application.createGameSnapshot().partners[0]==1,"name menu toggle on");
    tap(AppRenderer::partnerToggle());require(application.createGameSnapshot().partners[0]==0,"name menu toggle off");
    for(auto mode:{PlayerMode::TwoPlayers,PlayerMode::ThreePlayers,PlayerMode::FourPlayers}) {
        g.setPlayerMode(mode);
        for(std::size_t receiver=0;receiver<g.getPlayerCount();++receiver) {
            g.reset();g.setPartners(receiver,true);
            app::NavigationState nav;AppRenderer renderer(canvas);
            commander::CommanderDamageDraft d;
            screens::ScreenContext context{g,nav,d,renderer};
            screens::CommanderDamageScreen screen(context);
            nav.showCommanderDamage(receiver);screen.onEnter();
            auto selector=renderer.commanderSelector(receiver,receiver,g.getPlayerCount(),1);
            require(selector.x+selector.width<=372 || selector.x>=428 || selector.y+selector.height<=212 || selector.y>=268,"selectors must avoid centre menu");
            Point p{selector.x+selector.width/2,selector.y+selector.height/2};
            InputFrame frame;frame.gesture={TouchGesture::Tap,p,p};screen.handleInput(frame);
            require(d.selectedCommander(receiver)==1 && d.damageFrom(receiver)==0,"B selector does not edit damage");
            auto area=renderer.getPlayerRectangle(receiver,g.getPlayerCount());
            p=Point{area.x+(renderer.isPlayerUpsideDown(receiver,g.getPlayerCount())?30:area.width-30),area.y+area.height/2};
            frame.gesture={TouchGesture::Tap,p,p};screen.handleInput(frame);
            require(d.damageFrom(receiver)==1,"own B touch routing in each rotation");
            canvas.labels.clear();canvas.beginFrame();screen.draw();
#ifndef STICKY_HEADLESS_TEST
            if(receiver==0) {rlDrawRenderBatchActive();TakeScreenshot(("commander-partners-"+std::to_string(g.getPlayerCount())+".png").c_str());}
#endif
            canvas.endFrame();screen.onExit();
            require(g.getCommanderDamage(receiver,receiver,1)==1,"screen commits own B");
        }
    }
    std::cout<<"PASS: own commander, partner toggle, separate A/B lethal checks, migration, history, reset and all seat touch orientations.\n";
}



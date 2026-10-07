// Included after the simulator's recording canvas definition.
void runProfileTests(RecordedCanvas& canvas) {
    PlayerProfiles profiles;
    const char* expected[]={"Steve","Chris","Marvin","Marian","Erwin","Max","Fabi","Duki","David","Marcel","Emil"};
    require(profiles.size()==11,"eleven seeded profiles");
    for(int i=0;i<11;++i) require(std::string(profiles.at(i).name)==expected[i] && profiles.at(i).wins==0,"seed names and zero wins");
    require(profiles.assign(0,0,4) && !profiles.assign(1,0,4),"duplicate seating must be rejected");
    require(profiles.assign(3,1,4) && profiles.assign(1,1,2) && profiles.seat(3)==-1,"move inactive seat without duplicate");
    require(profiles.add("steve")==-1 && profiles.add("")==-1 && profiles.add(" ")==-1 && profiles.add("12345678901234567")==-1,"name validation");
    require(profiles.win(0) && profiles.at(0).wins==1 && profiles.undo() && !profiles.undo(),"undo exactly once");
    auto saved=profiles.snapshot(); auto bad=saved; bad.count=33;
    require(!profiles.restore(bad) && profiles.size()==11,"reject corrupt count without mutation");
    bad=saved; bad.seats[0]=99; require(!profiles.restore(bad),"reject invalid seat");
    bad=saved; bad.players[0].name[0]=0; require(!profiles.restore(bad),"reject empty name");
    bad=saved; std::memset(bad.players[0].name,'x',17); require(!profiles.restore(bad),"reject unterminated name");
    bad=saved; std::strcpy(bad.players[1].name,"Steve"); require(!profiles.restore(bad),"reject duplicate stored names");
    bad=saved; bad.lastWinner=0; require(!profiles.restore(bad),"reject invalid undo record");
    while(profiles.size()<32) require(profiles.add("Name"+std::to_string(profiles.size()))>=0,"profile capacity");
    require(profiles.add("Extra")==-1,"profile limit must be bounded");

    TestInput input;
    app::Application application(canvas,input);
    auto draw=[&](const char* shot=nullptr) {
        canvas.labels.clear(); canvas.beginFrame(); application.draw();
#ifndef STICKY_HEADLESS_TEST
        if(shot) {rlDrawRenderBatchActive();TakeScreenshot(shot);}
#else
        (void)shot;
#endif
        canvas.endFrame();
    };
    auto tap=[&](float x,float y) {
        input.next.gesture={TouchGesture::Tap,{x,y},{x,y}};
        application.tick(); application.checkTimeouts(); draw();
    };
    auto rectTap=[&](Rect r){tap(r.x+r.width/2,r.y+r.height/2);};
    auto nameTap=[&](int seat,int count=4){rectTap(AppRenderer::playerNameArea(seat,count));};
    auto menu=[&](){tap(400,240);};
    auto scores=[&](){menu();rectTap(AppRenderer::profilesButtonArea());};
    auto reset=[&](){menu();tap(150,410);};
    draw();
    const auto untouched=application.createGameSnapshot();
    nameTap(0); require(canvas.has("Select player",30),"tap name opens selector");
    draw("profiles-select.png");
    rectTap(AppRenderer::profileRow(0));
    require(application.profileSnapshot().seats[0]==0 && canvas.has("Steve",22,180),"assign name with rotation");
    require(application.createGameSnapshot().life==untouched.life,"select name preserves life");
    nameTap(1); rectTap(AppRenderer::profileRow(0));
    require(canvas.has("That player is already seated.",18),"duplicate assignment explains conflict");
    rectTap(AppRenderer::profileRow(1));
    nameTap(2);rectTap(AppRenderer::profileRow(2));
    nameTap(3);rectTap(AppRenderer::profileRow(3));
    draw("profiles-game-four.png");
    // Check actual regional redraw, before a full application.draw could mask an erased name.
    canvas.labels.clear();input.next.gesture={TouchGesture::Tap,{40,120},{40,120}};application.tick();
    require(canvas.has("Steve",22,180),"regional life update must redraw name");
    const auto changed=application.createGameSnapshot();
    reset(); require(canvas.has("Who won this game?",30),"reset opens winner prompt");
    draw("profiles-winner.png");
    application.rollDice();application.rollStartingPlayer();draw();
    require(canvas.has("Who won this game?",30),"hardware actions cannot dismiss winner prompt");
    rectTap(AppRenderer::profileFooter(2));require(application.createGameSnapshot().life==changed.life,"cancel reset preserves game");
    tap(645,410); // Settings Done.
    reset();rectTap(AppRenderer::profileRow(0));
    require(application.profileSnapshot().players[0].wins==1,"winner gets exactly one win");
    require(application.createGameSnapshot().life[0]==40 && application.createHistorySnapshot().count==0,"winner resets game and history");
    require(application.profileSnapshot().seats[0]==0,"reset keeps seats");
    rectTap(AppRenderer::profileRow(0)); // repeated release cannot award again
    require(application.profileSnapshot().players[0].wins==1,"winner input does not double count");
    auto sessionProfiles=application.profileSnapshot();auto sessionGame=application.createGameSnapshot();
    app::Application restarted(canvas,input);
    require(restarted.restoreGameSnapshot(sessionGame) && restarted.restoreProfiles(sessionProfiles),"restore game and profiles");
    require(restarted.profileSnapshot().players[0].wins==1 && restarted.profileSnapshot().seats[0]==0,"persistent scores and seat assignments");
    reset();rectTap(AppRenderer::profileFooter(0));
    require(application.profileSnapshot().players[0].wins==1,"no winner leaves scores alone");
    scores();draw("profiles-scores.png");
    rectTap(AppRenderer::profileFooter(1));require(canvas.has("Undo last recorded win?",30),"undo confirmation");
    rectTap(AppRenderer::profileFooter(0));
    require(application.profileSnapshot().players[0].wins==0 && application.profileSnapshot().lastWinner==-1,"undo removes one win only");
    rectTap(AppRenderer::profileNav(2)); rectTap(AppRenderer::profileNav(2));
    require(canvas.has("Emil   0 wins",24),"last page contains final seed");
    rectTap(AppRenderer::profileFooter(0));draw("profiles-keyboard.png");
    // Enter Zoe with automatic lowercase after first letter.
    rectTap(AppRenderer::keyboardKey(2,0));rectTap(AppRenderer::keyboardKey(0,8));rectTap(AppRenderer::keyboardKey(0,2));
    rectTap(AppRenderer::profileFooter(0));
    require(std::string(application.profileSnapshot().players[11].name)=="Zoe","on-screen keyboard creates player");
    rectTap(AppRenderer::profileRow(1)); // Rename Zoe on page 3.
    rectTap(AppRenderer::profileNav(2)); // backspace e
    rectTap(AppRenderer::keyboardKey(0,2)); // uppercase E
    rectTap(AppRenderer::profileFooter(0));
    require(std::string(application.profileSnapshot().players[11].name)=="ZoE","rename stored profile");
    rectTap(AppRenderer::profileFooter(2));tap(645,410);
    nameTap(3);rectTap(AppRenderer::profileFooter(1));
    require(application.profileSnapshot().seats[3]==-1,"Guest clears assignment");
    reset();rectTap(AppRenderer::profileRow(3));
    require(application.profileSnapshot().lastWinner==-1,"Guest result does not create win");
    for(auto mode:{PlayerMode::ThreePlayers,PlayerMode::TwoPlayers}) {
        auto game=application.createGameSnapshot();game.settings.playerMode=mode;
        require(application.restoreGameSnapshot(game),"layout restore");
        const int count=mode==PlayerMode::ThreePlayers?3:2;
        draw(count==3?"profiles-game-three.png":"profiles-game-two.png");
        for(int i=0;i<count;++i) {nameTap(i,count);require(canvas.has("Select player",30),"name tappable in each layout");rectTap(AppRenderer::profileFooter(2));}
        reset();require(!canvas.has("Player 4",24),"winner list limited to current seats");rectTap(AppRenderer::profileFooter(2));tap(645,410);
    }
    std::cout<<"PASS: seeded profiles, unique seating, keyboard, rename, pagination, guest, winner/reset/cancel, undo, persistence validation and 2/3/4-player names.\n";
}

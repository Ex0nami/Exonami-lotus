#include "sticky_lotus/ui/AppRenderer.h"
#include <algorithm>
namespace sticky_lotus::ui {
void AppRenderer::drawProfiles(const GameState& game,const ProfileView& v) {
    using Mode=ProfileView::Mode;
    canvas_.clear(Ink::White);
    auto text=[&](std::string_view s,Rect a,int size=24) {canvas_.drawText(s,a,size,Ink::Black,TextAlignment::Center,0);};
    auto button=[&](Rect a,const std::string& s,bool selected=false) {drawSettingsButton(a,s.c_str(),selected);};
    if(v.mode==Mode::Winner) {
        text("Who won this game?",{30,20,740,48},30);
        for(std::size_t i=0;i<game.getPlayerCount();++i)
            button(profileRow(i),game.profiles.seatName(i)+(game.profiles.seat(i)<0 ? " (guest)" : ""));
        text("Select a winner to record the result and reset.",{30,300,740,35},20);
        button(profileFooter(0),"No winner / reset"); button(profileFooter(2),"Cancel");
    } else if(v.mode==Mode::Undo) {
        text("Undo last recorded win?",{30,40,740,50},30);
        if(game.profiles.canUndo()) {
            const auto& winner=game.profiles.at(game.profiles.snapshot().lastWinner);
            text(winner.name,{30,140,740,60},36);
            text("Remove one win. Current game stays unchanged.",{30,230,740,40},22);
            button(profileFooter(0),"Undo win");
        }
        button(profileFooter(2),"Cancel");
    } else if(v.mode==Mode::Keyboard) {
        text(v.editing<0 ? "New player" : "Rename player",{30,10,740,40},28);
        text(v.text.empty()?"Enter name (max 16)":v.text,{40,60,720,50},30);
        text(v.error,{30,112,740,24},18);
        const char* rows[]={"QWERTYUIOP","ASDFGHJKL-","ZXCVBNM'  ","0123456789"};
        for(int row=0;row<4;++row) for(int col=0;col<10;++col) {
            char c=rows[row][col]; if(c==' ') continue; if(!v.upper && c>='A' && c<='Z') c+=32;
            button(keyboardKey(row,col),std::string(1,c));
        }
        button(profileNav(0),v.upper?"abc":"ABC"); button(profileNav(1),"Space"); button(profileNav(2),"Delete");
        button(profileFooter(0),"Save",true); button(profileFooter(2),"Cancel");
    } else {
        const bool select=v.mode==Mode::Select;
        text(select?"Select player":"Players & scores",{30,10,740,42},30);
        text(v.error.empty()?(select?"Choose a name for this seat":"Tap a name to rename"):v.error,{30,52,740,28},18);
        for(int row=0;row<5;++row) {
            const int id=v.page*5+row; if(id>=static_cast<int>(game.profiles.size())) break;
            const auto& p=game.profiles.at(id); std::string label=p.name;
            if(select && game.profiles.used(id,v.seat,game.getPlayerCount())) label+=" (seated)";
            else if(!select) label+="   "+std::to_string(p.wins)+(p.wins==1?" win":" wins");
            button(profileRow(row),label,select && game.profiles.seat(v.seat)==id);
        }
        if(v.page>0) button(profileNav(0),"Previous");
        text(std::to_string(v.page+1)+" / "+std::to_string(std::max<std::size_t>(1,(game.profiles.size()+4)/5)),profileNav(1),20);
        if((v.page+1)*5<static_cast<int>(game.profiles.size())) button(profileNav(2),"Next");
        if(game.profiles.size()<32) button(profileFooter(0),"New player");
        if(select) button(profileFooter(1),"Guest");
        else if(game.profiles.canUndo()) button(profileFooter(1),"Undo last win");
        button(profileFooter(2),"Back");
    }
    canvas_.invalidate({0,0,800,480}); canvas_.flush();
}
}

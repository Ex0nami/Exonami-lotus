#pragma once
#include "sticky_lotus/GameState.h"
#include <algorithm>
#include <array>
#include <chrono>
namespace sticky_lotus::commander {
class CommanderDamageDraft {
public:
    static constexpr std::size_t maximumPlayerCount=4;
    void begin(const GameState& game, std::size_t receiver) {
        receivingPlayer_=receiver; playerCount_=game.getPlayerCount(); active_=receiver<playerCount_;
        original_.fill(0); edited_.fill(0); delta_.fill(0); highlight_.fill(false); selected_.fill(0); partners_.fill(false);
        if(!active_) return;
        for(std::size_t a=0;a<playerCount_;++a) {
            partners_[a]=game.hasPartners(a);
            for(std::size_t c=0;c<2;++c) original_[a*2+c]=edited_[a*2+c]=game.getCommanderDamage(a,receiver,c);
        }
    }
    bool hasPartners(std::size_t a) const {return a<playerCount_ && partners_[a];}
    std::size_t selectedCommander(std::size_t a) const {return a<playerCount_ ? selected_[a] : 0;}
    void selectCommander(std::size_t a,std::size_t c) {if(active_ && a<playerCount_ && c<2 && (c==0 || partners_[a])) selected_[a]=c;}
    void changeDamage(std::size_t a,int amount) {
        if(!active_ || a>=playerCount_) return;
        const auto i=index(a); const int before=edited_[i];
        edited_[i]=static_cast<int>(std::clamp<long long>(static_cast<long long>(before)+amount,0,2147483647));
        delta_[i]+=edited_[i]-before; highlight_[i]=delta_[i]!=0;
        timestamp_[i]=std::chrono::steady_clock::now();
    }
    bool hasDamageDeltaHighlight(std::size_t a) const {return a<playerCount_ && highlight_[index(a)];}
    int getDamageDelta(std::size_t a) const {return a<playerCount_ ? delta_[index(a)] : 0;}
    std::chrono::steady_clock::time_point getDamageDeltaTimestamp(std::size_t a) const {return a<playerCount_ ? timestamp_[index(a)] : std::chrono::steady_clock::time_point{};}
    void clearDamageDeltaHighlight(std::size_t a) {if(a<playerCount_) {delta_[index(a)]=0;highlight_[index(a)]=false;}}
    void commit(GameState& game) {
        if(!active_) return;
        for(std::size_t a=0;a<playerCount_;++a) for(std::size_t c=0;c<(partners_[a]?2u:1u);++c) {
            const int difference=edited_[a*2+c]-original_[a*2+c];
            if(difference) game.changeCommanderDamage(a,receivingPlayer_,difference,c);
        }
        active_=false;
    }
    void cancel() {active_=false;}
    bool isActive() const {return active_;}
    std::size_t receivingPlayer() const {return receivingPlayer_;}
    std::size_t playerCount() const {return playerCount_;}
    int damageFrom(std::size_t a) const {return a<playerCount_ ? edited_[index(a)] : 0;}
    bool isLethal() const {
        if(!active_) return false;
        for(std::size_t a=0;a<playerCount_;++a) for(std::size_t c=0;c<(partners_[a]?2u:1u);++c)
            if(edited_[a*2+c]>=21) return true;
        return false;
    }
private:
    std::size_t index(std::size_t a) const {return a*2+selected_[a];}
    bool active_=false;
    std::size_t receivingPlayer_=0,playerCount_=0;
    std::array<bool,4> partners_{};
    std::array<std::size_t,4> selected_{};
    std::array<int,8> original_{},edited_{},delta_{};
    std::array<bool,8> highlight_{};
    std::array<std::chrono::steady_clock::time_point,8> timestamp_{};
};
}

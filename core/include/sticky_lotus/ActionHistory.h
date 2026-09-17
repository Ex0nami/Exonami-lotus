#pragma once
#include <array>
#include <chrono>
#include <cstdint>
#include <string>

namespace sticky_lotus {
enum class HistoryKind : std::uint32_t { Life, Poison, Commander, Ping, UndoPing, Dice, FirstPlayer };
struct HistoryEntry {
    HistoryKind kind{};
    std::int32_t player = 0, other = 0, before = 0, after = 0;
    std::uint32_t affected = 0;
};
struct HistorySnapshot {
    static constexpr std::uint32_t validMagic = 0x48495331; // HIS1
    std::uint32_t magic = validMagic, count = 0;
    std::array<HistoryEntry,10> entries{};
};
static_assert(sizeof(HistoryEntry)==24 && sizeof(HistorySnapshot)==248);
class ActionHistory {
public:
    using Clock = std::chrono::steady_clock;
    void clear() { data_ = {}; finishBurst(); }
    void finishBurst() { canMerge_ = false; }
    std::size_t size() const { return data_.count; }
    const HistoryEntry& entry(std::size_t newestIndex) const { return data_.entries.at(newestIndex); }
    HistorySnapshot snapshot() const { return data_; }
    bool restore(const HistorySnapshot& saved) {
        clear();
        if(saved.magic != HistorySnapshot::validMagic || saved.count > 10) return false;
        for(std::size_t i=0;i<saved.count;++i) {
            const auto& e=saved.entries[i];
            if(static_cast<std::uint32_t>(e.kind)>static_cast<std::uint32_t>(HistoryKind::FirstPlayer) ||
               e.player<0 || e.player>3 || e.other<0 || e.other>20 || e.before<0 || e.after<0 || e.affected>15) return false;
            if(e.kind==HistoryKind::Commander && (e.other>3 || e.other==e.player)) return false;
            if(e.kind==HistoryKind::Dice && ((e.other!=2 && e.other!=6 && e.other!=20) || e.after<1 || e.after>e.other)) return false;
            if((e.kind==HistoryKind::Ping || e.kind==HistoryKind::UndoPing) &&
               (e.affected==0 || (e.affected & (1u<<e.player)))) return false;
        }
        data_=saved; return true;
    }
    void add(HistoryEntry e, bool mergeLife=false, Clock::time_point now=Clock::now()) {
        if(mergeLife && canMerge_ && data_.count && e.kind==HistoryKind::Life &&
           data_.entries[0].kind==HistoryKind::Life && data_.entries[0].player==e.player &&
           data_.entries[0].after==e.before && now-lastLife_ < std::chrono::seconds(4)) {
            data_.entries[0].after=e.after;
        } else {
            const auto last=data_.count<10 ? data_.count++ : 9;
            for(std::size_t i=last;i>0;--i) data_.entries[i]=data_.entries[i-1];
            data_.entries[0]=e;
        }
        canMerge_=mergeLife;
        lastLife_=now;
    }
    std::string text(std::size_t index) const {
        const auto& e=entry(index);
        const std::string p="P"+std::to_string(e.player+1);
        const auto delta=static_cast<long long>(e.after)-e.before;
        const std::string change=(delta>0 ? "+" : "")+std::to_string(delta);
        switch(e.kind) {
        case HistoryKind::Life: return p+" Life: "+std::to_string(e.before)+" -> "+std::to_string(e.after)+" ("+change+")";
        case HistoryKind::Poison: return p+" Poison: "+std::to_string(e.before)+" -> "+std::to_string(e.after);
        case HistoryKind::Commander: return "P"+std::to_string(e.other+1)+" -> "+p+" Commander: "+std::to_string(e.before)+" -> "+std::to_string(e.after);
        case HistoryKind::Ping:
        case HistoryKind::UndoPing: {
            std::string targets;
            for(int i=0;i<4;++i) if(e.affected & (1u<<i)) targets+=(targets.empty() ? "" : ", ")+std::string("P")+std::to_string(i+1);
            return p+(e.kind==HistoryKind::Ping ? " Ping: " : " Undo ping: ")+targets+(e.kind==HistoryKind::Ping ? " -1" : " +1");
        }
        case HistoryKind::Dice: return e.other==2 ? (e.after==1 ? "Coin: HEADS" : "Coin: TAILS") : "D"+std::to_string(e.other)+": "+std::to_string(e.after);
        case HistoryKind::FirstPlayer: return p+" starts the game";
        }
        return {};
    }
private:
    HistorySnapshot data_{};
    bool canMerge_=false;
    Clock::time_point lastLife_{};
};
}

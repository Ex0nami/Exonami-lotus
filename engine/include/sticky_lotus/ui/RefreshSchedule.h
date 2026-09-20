#pragma once
#include <cstdint>

namespace sticky_lotus::ui {
// Pure timing policy shared with host tests. All times are monotonic microseconds.
class RefreshSchedule {
public:
    static constexpr std::int64_t quietUs = 35'000;
    static constexpr std::int64_t maximumBatchUs = 100'000;
    static constexpr std::int64_t cleanupIdleUs = 3'000'000;
    static constexpr std::int64_t maximumCleanupWaitUs = 20'000'000;
    static constexpr std::int64_t retryDelayUs = 1'000'000;
    void changed(std::int64_t now) {
        if (!pending_) pendingSince_=now;
        pending_=true; lastChange_=now;
    }
    bool pending() const { return pending_; }
    bool canAttempt(std::int64_t now) const { return now>=retryAt_; }
    bool partialDue(std::int64_t now) const {
        return pending_ && (now-lastChange_>=quietUs || now-pendingSince_>=maximumBatchUs);
    }
    bool fullDue(std::int64_t now) const {
        return fullPending_ && (forceFull_ || now-lastChange_>=cleanupIdleUs || now-fullSince_>=maximumCleanupWaitUs);
    }
    void partialComplete(std::int64_t now, bool needsFull) {
        pending_=false; retryAt_=0;
        if (needsFull && !fullPending_) { fullPending_=true; fullSince_=now; }
        if (!needsFull) { fullPending_=false; forceFull_=false; }
    }
    void fullComplete() { pending_=false; fullPending_=false; forceFull_=false; retryAt_=0; }
    void failed(std::int64_t now, bool full) {
        retryAt_=now+retryDelayUs;
        if (full) { fullPending_=true; forceFull_=true; }
    }
private:
    bool pending_=false, fullPending_=false, forceFull_=false;
    std::int64_t lastChange_=0, pendingSince_=0, fullSince_=0, retryAt_=0;
};
}

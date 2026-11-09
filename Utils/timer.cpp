#include "Seq-Thread-OMP/workbench.h"
#include <chrono>
#include <algorithm>
#include <numeric>

using namespace std;

double timer_now_ms() {
    auto now = chrono::steady_clock::now();
    chrono::duration<double,milli> duration = now.time_since_epoch();
    return duration.count();
}

TimingStats summarize_times(const vector<double>& times) {
    TimingStats stats{};
    if (times.size() == 0) {
        return stats;
    }

    stats.best_ms = times[0];
    stats.worst_ms = times[0];
    stats.avg_ms = 0;
    for (size_t i = 0; i < times.size(); i++) {
        stats.avg_ms = stats.avg_ms + times[i];
        if (times[i] > stats.worst_ms) {
            stats.worst_ms = times[i];
        }
        if (times[i] < stats.best_ms) {
            stats.best_ms = times[i];
        }
    }
    stats.avg_ms = stats.avg_ms / times.size();
    return stats;
}
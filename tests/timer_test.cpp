#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include "workbench.h"

using namespace std;

int main() {
    double start = timer_now_ms();
    this_thread::sleep_for(chrono::milliseconds(100));
    double end = timer_now_ms();

    cout << "Elapsed: " << end-start << endl;

    vector<double> times = {10.0, 20.0, 30.0};
    TimingStats stats = summarize_times(times);

    cout << "Average: " << stats.avg_ms << endl;
    cout << "Best: " << stats.best_ms << endl;
    cout << "Worst: " << stats.worst_ms << endl;

    return 0;
}
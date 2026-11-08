#include "output.h"
#include <iostream>
#include <iomanip>
#include <sstream>

//prints required status line
void print_status(const string& status){
    cout << "STATUS: " << status << endl;
}

//turns run's ans into txt for RESULT line
string format_result(const input& cfg, const WorkloadResult& res){
    ostringstream out;
    if(cfg.workload == LOOP){
        out << res.processed;   //num of elements written
    }else if(cfg.workload == DOT){
        out << fixed << setprecision(6) << res.value;   //dot product
    }else{
        out << fixed << setprecision(8) << res.value;   //pi estimate
    }
    return out.str();
}

//turns run's check val into txt for CHECKSUM line
string format_checksum(const input& cfg, const WorkloadResult& res){
    ostringstream out;
    if(cfg.workload == DOT){
        out << res.processed;
    }else{
        out << res.checksum;
    }
    return out.str();
}

//prints full report for SEQUENTIAL || STD_THREAD || OPENMP
void print_single_report(const input& cfg, const WorkloadResult& res, 
    const TimingStats& stats, bool valid){
    string schedule = schedule_to_string(cfg.schedule);
    if(cfg.mode == SEQUENTIAL){
        schedule = "NONE";
    }

    cout << "STATUS: OK" << endl;
    cout << "WORKLOAD: " << workload_to_string(cfg.workload) << endl;
    cout << "MODE: " << mode_to_string(cfg.mode) << endl;
    cout << "SIZE: " << cfg.size << endl;
    cout << "WORKERS: " << res.workers_used << endl;
    cout << "SCHEDULE: " << schedule << endl;
    cout << "RESULT: " << format_result(cfg, res) << endl;
    cout << "CHECKSUM: " << format_checksum(cfg, res) << endl;
    cout << fixed << setprecision(3);
    cout << "AVG_TIME_MS: " << stats.avg_ms << endl;
    cout << "BEST_TIME_MS: " << stats.best_ms << endl;
    cout << "WORST_TIME_MS: " << stats.worst_ms << endl;
    cout << "REPETITIONS: " << cfg.repetitions << endl;
    cout << "VALID: " << (valid ? "YES" : "NO") << endl;
}

//header column titles for COMPARE table, but milestone 3
void print_compare_header() {
    cout << left
         << setw(17) << "VERSION"
         << setw(17) << "ACTIVE_WORKERS"
         << setw(22) << "RESULT"
         << setw(22) << "CHECKSUM"
         << setw(12) << "AVG_MS"
         << setw(12) << "BEST_MS"
         << setw(12) << "WORST_MS"
         << setw(10) << "SPEEDUP"
         << "EFFICIENCY" << endl;
}

//prints one row in COMPARE table
void print_compare_row(const string& version, int active_workers,
    const string& result, const string& checksum, 
    const TimingStats& stats, double speedup, double efficiency){
    cout << left << fixed << setprecision(3)
         << setw(17) << version
         << setw(17) << active_workers
         << setw(22) << result
         << setw(22) << checksum
         << setw(12) << stats.avg_ms
         << setw(12) << stats.best_ms
         << setw(12) << stats.worst_ms
         << setw(10) << speedup
         << efficiency << endl;
    cout.unsetf(ios::fixed);
    cout << right << setprecision(6);
}
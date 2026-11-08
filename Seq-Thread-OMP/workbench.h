#ifndef WORKBENCH_H
#define WORKBENCH_H

#include <vector>
#include "input.h"

using namespace std;

//arrays workloads use
struct WorkloadData {
    vector<unsigned long long> loop_out; //LOOP: output array, 1 val per index
    vector<double> a;   //DOT: 1st input arr
    vector<double> b;   //DOT: 2nd input arr
};

//one run of model returns so main can validate & print
struct WorkloadResult {
    double value;   //DOT: dot product, PI: pi estimate
    unsigned long long checksum;    //LOOP: weighted checksum, PI: points inside circle
    unsigned long long processed;   //num of indices/trials done, should == SIZE
    int workers_used;   //threads ran, SEQUENTIAL should == 1
};

//summary of all repetition times in milliseconds
struct TimingStats {
    double avg_ms;      //average
    double best_ms;     //fastest
    double worst_ms;    //slowest
};

// TODO ALL FUNCS BELOW
//allocate and fill arrays for chosen workload
void setup_workload(const input& cfg, WorkloadData& data);
//run workload with one thread
WorkloadResult run_sequential(const input& cfg, WorkloadData& data);
//return true if run matches sequential ans
bool validate_result(const input& cfg, const WorkloadResult& run, const WorkloadResult& reference);
//run workload with threads using chosen schedule
WorkloadResult run_std_thread(const input& cfg, WorkloadData& data);
//run workload with openMP using chosen schedule
WorkloadResult run_openmp(const input& cfg, WorkloadData& data);
//return current steady_clock time in milliseconds
double timer_now_ms();
//compute average, best, worst from list of repetition times
TimingStats summarize_times(const vector<double>& times_ms);

#endif //WORKBENCH_H
#include <iostream> //cin, cout, cerr
#include <omp.h> // brew install libomp for Mac, toolchain will need to be updated if on CLION
#include <thread>   //thread::hardware_concurrency to count cores
#include <string>
//#include <sstream>
//#include <iomanip>
#include <vector>
#include <new>  //bad_alloc

#include "input.h"  //input struct and parse_input()
#include "workbench.h"  //shared structs and TODO functions
#include "output.h" //print_status() and print single report
#include "thread.h"

using namespace std;

//main is complete but right now it doesn't run 
//because the functions in workbench.h still need to be implemented.
//in meantime currently compiles but doesnt link

//returns true only for current runnable modes on this build
static bool mode_supported_in_this_build(const input& cfg){
    return cfg.mode == SEQUENTIAL || cfg.mode == STD_THREAD || cfg.mode == OPENMP;
}

//runs model picked in MODE
static WorkloadResult run_selected_model(const input& cfg, WorkloadData& data){
    if(cfg.mode == STD_THREAD){
        return run_std_thread(cfg, data);   //threads version   
    }
    if(cfg.mode == OPENMP){
        return run_openmp(cfg, data);       //openMP version
    }
    return run_sequential(cfg, data);       //sequential (one thread) version
}
//clear LOOP array before each run
static void reset_outputs(const input& cfg, WorkloadData& data){
    if(cfg.workload == LOOP){
        for(size_t i = 0; i < data.loop_out.size(); i++){
            data.loop_out[i] = 0;
        }
    }
}

int main() {
    input config;   //settings read from input file
    string err;

    //1. read and check input
    if(!parse_input(cin, config, err)){
        print_status("INVALID_INPUT");
        cerr << "Reason: " << err << endl;
        return 1;
    }

    //2. check mode
    if(!mode_supported_in_this_build(config)){
        print_status("UNSUPPORTED_MODE");
        cerr << "Reason: MODE " << config.mode_str << " is not available in the Seq-Thread-OMP build." << endl;
        return 2;
    }

    //threads num > cores num
    unsigned int cores = thread::hardware_concurrency();
    if(config.mode != SEQUENTIAL && cores != 0 && (unsigned int) config.threads > cores){
        cerr << "WARNING: THREADS " << config.threads << " is more than the ";
        cerr << cores << " hardware threads on this machine (oversubscribed)." << endl;
    }

    WorkloadData data;  //arrays workload uses
    WorkloadResult reference;   //correct ans from sequential version
    try{
        //3. build arrays, not timed
        setup_workload(config, data);
        //4. get correct ans to compare against, not timed
        reference = run_sequential(config, data);
    }catch(const bad_alloc&){
        print_status("INVALID_INPUT");
        cerr << "Reason: not enough memory for SIZE " << config.size << endl;
        return 1;
    }

    vector<double> times_ms;    //time of each repetition in milliseconds
    WorkloadResult last_result = reference;
    bool all_valid = true;

    //5. run model picked REPETITION num times and time each run
    for(int rep = 0; rep < config.repetitions; rep++){
        reset_outputs(config, data);    //clear old results

        double start_ms = timer_now_ms();
        WorkloadResult res = run_selected_model(config, data);
        double end_ms = timer_now_ms();

        times_ms.push_back(end_ms - start_ms);

        if(!validate_result(config, res, reference)){
            all_valid = false;
            cerr << "Repetition " << (rep + 1) << " did not match the sequential reference." << endl;
        }
        last_result = res;
    }

    //6. report avg best worst times
    TimingStats stats = summarize_times(times_ms);
    print_single_report(config, last_result, stats, all_valid);

    return 0;
}
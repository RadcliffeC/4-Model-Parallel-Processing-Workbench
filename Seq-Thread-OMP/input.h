//
// Created by Chandler Radcliffe on 11/5/26.
// modified by Sofia 11/7/26.
//

#ifndef INC_4_MODEL_PARALLEL_PROCESSING_WORKBENCH_INPUT_H
#define INC_4_MODEL_PARALLEL_PROCESSING_WORKBENCH_INPUT_H

using namespace std;
#include <string>
#include <iostream>

// WORKLOAD workload
// MODE mode
// [MODEL model]
// SIZE n
// THREADS t
// PROCESSES p
// SCHEDULE schedule
// CHUNK c
// SEED s
// REPETITIONS r

enum SCHEDULE { STATIC, CYCLIC, DYNAMIC };
enum MODE { SEQUENTIAL, STD_THREAD, OPENMP, MPI, COMPARE, TRACE };
enum WORKLOAD { LOOP, DOT, PI };
enum MODEL { MODEL_NONE, MODEL_STD_THREAD, MODEL_OPENMP, MODEL_MPI };

//arbitary nums can change, not specified in spec yet
//acceptable limit max values from input file
//MAX_SIZE(n)
const long long MAX_SIZE = 1000000000LL;    //should be between 10^7 and 10^9, overflow
const int MAX_THREADS = 256;
const int MAX_PROCESSES = 256;
const int MAX_REPETITIONS = 256;

/*
 * struct to parse input
 */
struct input {
    WORKLOAD workload;
    string workload_str;

    MODE mode;
    string mode_str;

    MODEL model;       // only required when mode is TRACE: STD_THREAD, OPENMP, or MPI
    string model_str;

    long long size;    // num loop iterations, array elements or mc points (n > 0)
    int threads;       // number of c++11 or omp threads (t > 0)
    int processes;     // MPI communicator size for MPI and compare modes (p >= 2 for dynamic/compare, p >= 1 for mpi, ignored otherwise)

    SCHEDULE schedule;
    string schedule_str;

    int chunk;         // chunk size for cyclic and dynamic scheduling (c > 0, ignored for static)
    unsigned long long seed; // base random seed
    int repetitions;   // number of repeated timing runs (r > 0)
};

bool parse_workload(const string& s, WORKLOAD& w);
bool parse_mode(const string& s, MODE& m);
bool parse_model(const string& s, MODEL& m);
bool parse_schedule(const string& s, SCHEDULE& sch);

string workload_to_string(WORKLOAD w);
string mode_to_string(MODE m);
string model_to_string(MODEL m);
string schedule_to_string(SCHEDULE sch);

bool validate_config(const input& in, string& err);
bool validate_mpi_comm_size(const input& in, int actual_comm_size, string& err);
bool parse_input(istream& is, input& in, string& err);
void print_config(const input& in);

#endif //INC_4_MODEL_PARALLEL_PROCESSING_WORKBENCH_INPUT_H


#ifndef OUTPUT_H
#define OUTPUT_H

#include <string>
#include "input.h"
#include "workbench.h"

using namespace std;

void print_status(const string& status);

string format_result(const input& cfg, const WorkloadResult& res);
string format_checksum(const input& cfg, const WorkloadResult& res);

void print_single_report(const input& cfg, const WorkloadResult& res, 
    const TimingStats& stats, bool valid);

void print_compare_header();
void print_compare_row(const string& version, int active_workers, 
    const string& result, const string& checksum, const TimingStats& stats, 
    double speedup, double efficiency);

#endif //OUTPUT_H
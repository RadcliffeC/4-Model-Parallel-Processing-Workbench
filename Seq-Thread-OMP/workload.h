//
// Created by Zack Govin on 11/8/2026.
//

#ifndef WORKLOAD_H
#define WORKLOAD_H

#include <vector>
#include "input.h"
#include "workbench.h"

using namespace std;

typedef unsigned long long u64;

inline u64 loop_value(u64 i);

// Checksum term for one element
inline u64 loop_weight(u64 i, u64 value);

// Fills data.loop_out and returns the checksum of that range
u64 loop_range(vector<u64>& out, u64 lo, u64 hi);

// DOT
inline double dot_a(u64 i);
inline double dot_b(u64 i);

// Returns sum of a[i] * b[i] over [lo, hi]
double dot_range(const vector<double>& a, const vector<double>& b, u64 lo,
    u64 hi);

// PI
inline u64 splitmix64_at(u64 seed, u64 k);

// Top 53 bits -> uniform double in [0, 1)
inline double unit_double(u64 seed, u64 k);

inline bool pi_point_inside(u64 seed, u64 i);

// Returns how many points land inside the quarter circle
u64 pi_range(u64 seed, u64 lo, u64 hi);

// pi estimate from a total inside count
inline double pi_estimate(u64 inside, u64 n);

// Extra checks
bool validate_loop_array(const WorkloadData& data, long long n, string& err);

// Partition coverage check for scheduler tests
bool validate_visit_counts(const vector<int>& visits, string& err);

// How many standard errors the estimate is from the real pi
double pi_standard_errors_from_pi(double estimate, u64 n);

#endif WORKLOAD_H

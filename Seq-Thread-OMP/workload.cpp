//
// Created by Zack Govin on 11/8/2026.
//

#include "workload.h"
#include <cmath>
#include <string>

using namespace std;

// LOOP
const u64 LOOP_MODULUS = 1000003ULL;

inline u64 loop_value(u64 i) {
    u64 r = i % LOOP_MODULUS;
    return (r * r + 3ULL * r + 7ULL) & LOOP_MODULUS;
}

// Checksum term for one element
inline u64 loop_weight(u64 i, u64 value) {
    return value * (i + 1ULL);
}

// Kernels
u64 loop_range(vector<u64>& out, u64 lo, u64 hi) {
    u64 checksum = 0; // identity
    for (u64 i = lo; i < hi; i++) {
        u64 v = loop_value(i);
        out[i] = v; // only this range's slots are written
        checksum += loop_weight(i, v); // wraps mod 2^64 by design
    }
    return checksum;
}

double dot_range(const vector<double>& a, const vector<double>& b, u64 lo,
    u64 hi) {
    double sum = 0.0; // identity
    for (u64 i = lo; i < hi; i++) {
        sum += a[i] * b[i];
    }
    return sum;
}

u64 pi_range(u64 seed, u64 lo, u64 hi) {
    u64 inside = 0; // identity
    for (u64 i = lo; i < hi; i++) {
        if (pi_point_inside(seed, i)) {
            inside++;
        }
    }
    return inside;
}

// DOT
inline double dot_a(u64 i) {
    return static_cast<double>((i % 1000ULL) + 1ULL) / 1000.0;
}
inline double dot_b(u64 i) {
    return 1.0 / static_cast<double>((i % 100ULL) + 1ULL);
}

// Floating point tolerance for DOT validation
const double DOT_REL_TOLERANCE = 1e-6;

// PI
inline u64 splitmix64_at(u64 seed, u64 k) {
    u64 z = seed + (k + 1ULL) * 0x9E3779B97F4A7C15ULL; // wraps mod 2^64
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

// Top 53 bits -> uniform double in [0, 1)
inline double unit_double(u64 seed, u64 k) {
    return static_cast<double>(splitmix64_at(seed, k) >> 11) * (1.0 / 9007199254740992.0);
}

inline bool pi_point_inside(u64 seed, u64 i) {
    double x = unit_double(seed, 2ULL * i);
    double y = unit_double(seed, 2ULL * i + 1ULL);
    return x * x + y * y <= 1.0;
}

// pi estimate from a total inside count
inline double pi_estimate(u64 inside, u64 n) {
    return 4.0 * static_cast<double>(inside) / static_cast<double>(n);
}

// Setup
void setup_workload(const input& cfg, WorkloadData& data) {
    u64 n = static_cast<u64>(cfg.size);

    data.loop_out.clear();
    data.a.clear();
    data.b.clear();

    if (cfg.workload == LOOP) {
        data.loop_out.assign(n, 0ULL);
    } else if (cfg.workload == DOT) {
        data.a.resize(n);
        data.b.resize(n);
        for (u64 i = 0; i < n; i++) {
            data.a[i] = dot_a(i);
            data.b[i] = dot_b(i);
        }
    }
}

// Sequential
WorkloadResult run_sequential(const input& cfg, WorkloadData& data) {
    u64 n = static_cast<u64>(cfg.size);
    WorkloadResult res;
    res.value = 0.0;
    res.checksum = 0;
    res.processed = n; // one range covering every index exactly once
    res.workers_used = 1;

    if (cfg.workload == LOOP) {
        res.checksum = loop_range(data.loop_out, 0, n);
    } else if (cfg.workload == DOT) {
        res.value = dot_range(data.a, data.b, 0, n); // inside count
        res.value = pi_estimate(res.checksum, n);
    }
    return res;
}

// Validation
bool validate_result(const input& cfg, const WorkloadResult& run,
    const WorkloadResult& reference) {
    bool returnFlag = false;
    u64 n = static_cast<u64>(cfg.size);
    if (run.processed != n || reference.processed != n) {
        returnFlag = false;
    }

    if (cfg.workload == LOOP) {
        returnFlag = (run.checksum == reference.checksum);
    }
    if (cfg.workload == DOT) {
        if (!isfinite(run.value) || !isfinite(reference.value)) {
            returnFlag = false;
        }
        double diff = fabs(run.value - reference.value);
        double scale = fmax(1.0, fabs(reference.value));
        returnFlag = (diff <= DOT_REL_TOLERANCE * scale);
    }
    if (cfg.workload == PI) {
        returnFlag = (run.checksum == reference.checksum);
    }
    return returnFlag;
}

// Extra checks
bool validate_loop_array(const WorkloadData& data, long long n, string& err) {
    bool returnFalseFlag = false;
    bool returnFlag = false;
    if (static_cast<long long>(data.loop_out.size()) != n) {
        err = "LOOP array has size " + to_string(data.loop_out.size()) +
            ", expected " + to_string(n);
        returnFalseFlag = true;
    }
    for (u64 i = 0; i < static_cast<u64>(n); i++) {
        u64 expected = loop_value(i);
        if (data.loop_out[i] != expected) {
            if (data.loop_out[i] == 0) {
                err = "index " + to_string(i) + " was never written";
            } else {
                err = "index " + to_string(i) + " holds " +
                    to_string(data.loop_out[i]) +
                        ", expected " + to_string(expected);
            }
            returnFalseFlag = true;
        }
    }
    if (returnFalseFlag) {
        returnFlag = false;
    } else {
        returnFlag = true;
    }
    return returnFlag;
}

bool validate_visit_counts(const vector<int>& visits, string& err) {
    bool returnFalseFlag = false;
    bool returnFlag = false;
    for (size_t i = 0; i < visits.size(); i++) {
        if (visits[i] != 1) {
            err = "index " + to_string(i) + " visited " + to_string(visits[i]) +
                " times (expected exactly 1)";
            returnFalseFlag = true;
        }
    }
    if (returnFalseFlag) {
        returnFlag = false;
    } else {
        returnFlag = true;
    }
    return returnFlag;
}

double pi_standard_errors_from_pi(double estimate, u64 n) {
    // Each point is inside with probability p = pi/4
    const double PI_TRUE = 3.14159265358979323846;
    double p = PI_TRUE / 4.0;
    double se = 4.0 * sqrt(p * (1.0 - p) / static_cast<double>(n));
    return fabs(estimate - PI_TRUE) / se;
}
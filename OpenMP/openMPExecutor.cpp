//
// Created by Lawrence O'Connor on 11/8/26.
//

#include "Seq-Thread-OMP/workload.h"
#include "Seq-Thread-OMP/workbench.h"
#include <omp.h>
#include <iostream>
#include <algorithm>

using namespace std;

WorkloadResult run_openmp(const input &cfg, WorkloadData &data) {
    WorkloadResult result{};
    const u64 n = static_cast<u64>(cfg.size);

    u64 checksum = 0;
    double dotSum = 0.0;
    int workers;

    #pragma omp parallel num_threads(cfg.threads) reduction(+:checksum, dotSum)
    {
        int threadId = omp_get_thread_num();
        int threadCount = omp_get_num_threads();

#pragma omp single
        {
            workers = threadCount;
        }
        u64 blockSize = n / threadCount;
        u64 remainder = n % threadCount;
        u64 start = threadId * blockSize + min<u64>(threadId, remainder);
        u64 end = start + blockSize;
        if (threadId < remainder) {
            end++;
        }

        if (cfg.workload == LOOP) {
            checksum += loop_range(data.loop_out, start, end);
        }
        else if (cfg.workload == DOT) {
            dotSum += dot_range(data.a,data.b, start, end);
        }
        else if (cfg.workload == PI) {
            checksum += pi_range(cfg.seed, start, end);
        }
    }
    result.processed = n;
    result.checksum = checksum;
    result.workers_used = workers;
    if (cfg.workload == DOT) {
        result.value = dotSum;
    }
    else if (cfg.workload == PI) {
        result.value = 4.0*static_cast<double>(checksum)/n;
    }
    return result;
}

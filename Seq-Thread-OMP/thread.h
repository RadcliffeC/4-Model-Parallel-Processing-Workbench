//
// Created by Chandler Radcliffe on 11/9/26.
//

#ifndef INC_4_MODEL_PARALLEL_PROCESSING_WORKBENCH_THREAD_H
#define INC_4_MODEL_PARALLEL_PROCESSING_WORKBENCH_THREAD_H
#include "input.h"
#include "workload.h"

#include <thread>
#include <atomic>
#include <stdexcept>
#include <algorithm>
#include <vector>

template<class Function>
int parallel_for_ranges(const input& cfg, Function function) {
    const long long n = cfg.size;

    const int numThreads = static_cast<int>(
        std::min(static_cast<long long>(cfg.threads), n)
    );

    std::vector<std::thread> workers;
    workers.reserve(numThreads);

    const long long chunk = std::max(1, cfg.chunk);
    std::atomic<long long> next_index(0);

    for (int tid = 0; tid < numThreads; tid++) {
        workers.emplace_back([&, tid]() {
            if (cfg.schedule == STATIC) {
                const long long start = (static_cast<long long>(tid) * n) / numThreads;
                const long long end = (static_cast<long long>(tid + 1) * n) / numThreads;

                if (start < end) {
                    function(tid, start, end);
                }
            } else if (cfg.schedule == CYCLIC) {
                for (long long start = static_cast<long long>(tid) * chunk;
                     start < n;
                     start += static_cast<long long>(numThreads) * chunk) {
                    const long long end = std::min(start + chunk, n);
                    function(tid, start, end);
                }
            } else if (cfg.schedule == DYNAMIC) {
                while (true) {
                    const long long start = next_index.fetch_add(
                        chunk,
                        std::memory_order_relaxed
                    );

                    if (start >= n) {
                        break;
                    }

                    const long long end = std::min(start + chunk, n);
                    function(tid, start, end);
                }
            }
        });
    }

    for (std::thread& worker : workers) {
        worker.join();
    }

    return numThreads;
}


WorkloadResult run_std_thread(const input& cfg, WorkloadData& data) {
    WorkloadResult result;

    result.value = 0.0;
    result.workers_used = 0;
    result.processed = 0;
    result.checksum = 0;

    const long long n = cfg.size;

    if (n <= 0 || cfg.threads <= 0) {
        throw std::invalid_argument("Invalid input");
    }

    const int numThreads = static_cast<int>(
        std::min(static_cast<long long>(cfg.threads), n)
    );

    if (cfg.workload == LOOP) {
        data.loop_out.resize(static_cast<size_t>(n));

        std::vector<u64> partial_checksums(numThreads, 0ULL);

        result.workers_used = parallel_for_ranges(
            cfg,
            [&](int tid, long long lo, long long hi) {
                partial_checksums[tid] += loop_range(
                    data.loop_out,
                    static_cast<u64>(lo),
                    static_cast<u64>(hi)
                );
            }
        );

        for (int tid = 0; tid < numThreads; tid++) {
            result.checksum += partial_checksums[tid];
        }

        result.processed = static_cast<u64>(n);
    } else if (cfg.workload == DOT) {
        if (data.a.size() < static_cast<size_t>(n) ||
            data.b.size() < static_cast<size_t>(n)) {
            throw std::invalid_argument("Insufficient data for DOT workload");
        }

        std::vector<double> partial_sums(numThreads, 0.0);

        result.workers_used = parallel_for_ranges(
            cfg,
            [&](int tid, long long lo, long long hi) {
                partial_sums[tid] += dot_range(
                    data.a,
                    data.b,
                    static_cast<u64>(lo),
                    static_cast<u64>(hi)
                );
            }
        );

        for (int tid = 0; tid < numThreads; tid++) {
            result.value += partial_sums[tid];
        }

        result.processed = static_cast<u64>(n);
    } else if (cfg.workload == PI) {
        std::vector<u64> partial_counts(numThreads, 0ULL);

        result.workers_used = parallel_for_ranges(
            cfg,
            [&](int tid, long long lo, long long hi) {
                partial_counts[tid] += pi_range(
                    cfg.seed,
                    static_cast<u64>(lo),
                    static_cast<u64>(hi)
                );
            }
        );

        for (int tid = 0; tid < numThreads; tid++) {
            result.checksum += partial_counts[tid];
        }

        result.processed = static_cast<u64>(n);
        result.value = 4.0 * static_cast<double>(result.checksum) /
                       static_cast<double>(n);
    } else {
        throw std::invalid_argument("Unknown workload");
    }

    return result;
}



#endif // INC_4_MODEL_PARALLEL_PROCESSING_WORKBENCH_THREAD_H

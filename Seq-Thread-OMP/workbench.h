#ifndef WORKBENCH_H
#define WORKBENCH_H

#include <vector>
#include <thread>
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

static unsigned long long splitmix64(unsigned long long x)
{
    x += 0x9E3779B97F4A7C15ULL;

    x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL;
    x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL;

    return x ^ (x >> 31);
}


static double random_coordinate(
    unsigned long long seed,
    unsigned long long index,
    unsigned long long stream)
{
    unsigned long long value =
        splitmix64(seed ^ splitmix64(index + stream));

    return static_cast<double>(value >> 11) *
           (1.0 / 9007199254740992.0);
}

// TODO ALL FUNCS BELOW
//allocate and fill arrays for chosen workload
void setup_workload(const input& cfg, WorkloadData& data);
//run workload with one thread
WorkloadResult run_sequential(const input& cfg, WorkloadData& data);
//return true if run matches sequential ans
bool validate_result(const input& cfg, const WorkloadResult& run, const WorkloadResult& reference);
//run workload with threads using chosen schedule

template<class function>
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

    //workload 1
    if (cfg.workload == LOOP) {
        const unsigned long long M = 1000000007ULL;

        data.loop_out.resize(n);

        std::vector<unsigned long long> checksums(
            numThreads, 0
            );

        result.workers_used = parallel_for(
            cfg, [&](int tid, long long i) {
            unsigned long long index =
                static_cast<unsigned long long>(i);

            unsigned long long value =
                (index * index + 3ULL * index + 7ULL + 1) % M;
            data.loop_out[i] = value;

            checksums[tid] += (index + 1ULL) * value;
        }
        );

        for (int i=0;i<numThreads;i++) {
            result.checksum += checksums[i];
        }
        result.processed = n;

    }

    else if (cfg.workload == DOT) {
        if (data.a.size() < static_cast<size_t>(n) ||
            data.b.size() < static_cast<size_t>(n)) {
            throw std::invalid_argument("Insufficient data for DOT workload");
        }

        std::vector<double> partial_sums(
            numThreads, 0.0);

        result.workers_used = parallel_for(
            cfg,
            [&](int tid, long long i) {
               partial_sums[tid] +=
                   data.a[i] * data.b[i];
            });

        for (int i=0;i<numThreads;i++) {
            result.value += partial_sums[i];
        }
        result.processed = n;
    }

    else if (cfg.workload == PI) {
        const unsigned long long X_STREAM =
            0x123456789ABCDEF0ULL;
        const unsigned long long Y_STREAM =
            0xFEDCBA9876543210ULL;

        std::vector<unsigned long long> partial_counts(
            numThreads, 0
        );

        result.workers_used = parallel_for(
            cfg,
            [&](int tid, long long i) {
               unsigned long long index =
                   static_cast<unsigned long long>(i);

                double x = random_coordinate(cfg.seed, index, X_STREAM);
                double y = random_coordinate(cfg.seed, index, Y_STREAM);

                if (x *x + y*y <= 1.0) {
                    partial_counts[tid]++;
                }
            });

        for (int i = 0; i < numThreads; i++) {
            result.checksum += partial_counts[i];
        }
        result.processed = n;

        result.value =
            4.0 * static_cast<double>(result.checksum) /
                static_cast<double>(n);

    }

    else {
        throw std::invalid_argument("Unknown workload");
    }
    return result;

    std::vector<std::thread> workers;
}



//run workload with openMP using chosen schedule
WorkloadResult run_openmp(const input& cfg, WorkloadData& data);
//return current steady_clock time in milliseconds
double timer_now_ms();
//compute average, best, worst from list of repetition times
TimingStats summarize_times(const vector<double>& times_ms);



template<class Function>
int paralell_for(const input& cfg, Function function) {
    long long n = cfg.size;

    int numThreads = static_cast<int>(
        std::min(static_cast<long long>(cfg.threads), n)
        );

    std::vector<std::thread> workers;

    // only for dynamic
    std::atomic<long long> next_index(0);

    int chunk = cfg.chunk;

    for (int i = 0; i < numThreads; i++) {
        workers.emplace_back([&,i]() {
            if (cfg.schedule == STATIC) {
                long long start = (i * n) / numThreads;
                long long end = ((i + 1) * n) / numThreads;

                for (long long j = start; j < end; j++) {
                   function(i,j);
                }
            }
            else if (cfg.schedule == CYCLIC){
                for (long long start = i * chunk;
                    start < n;
                    start += static_cast<long long>(numThreads) * chunk) {
                    long long end = std::min(start + chunk, n);

                    for (long long n = start; n<end; n++) {
                        function(i,n);
                    }
                }
            }

            else if (cfg.schedule == DYNAMIC) {
                while (true)
                {
                    long long start = next_index.fetch_add(
                        chunk, std::memory_order_relaxed
                    );

                    if (start >= n)
                        break;

                    long long end = std::min(
                        start + chunk, n
                    );

                    for (long long x = start; x < end; x++)
                        function(i, x);
                }
            }

        });

    }

    for (size_t i = 0; i<workers.size(); i++) {
        workers[i].join();
    }

    return numThreads;

}



#endif //WORKBENCH_H
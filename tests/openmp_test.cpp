//
// Created by Lawrence O'Connor on 11/8/26.
//

#include <iostream>

#include "workbench.h"
#include "workload.h"

using namespace std;

int main() {
    //Test Config
    input cfg{};
    cfg.threads = 4;
    cfg.workload = LOOP;
    cfg.size = 1000;
    cfg.schedule = STATIC;
    cfg.chunk = 100;
    cfg.seed = 12345;
    cfg.repetitions = 1;

    //Test Data
    WorkloadData data{};


    //Run OpenMP workloads
    setup_workload(cfg, data);
    WorkloadResult loopResult = run_openmp(cfg, data);
    setup_workload(cfg, data);
    WorkloadResult loopSeq = run_sequential(cfg, data);
    cfg.workload = DOT;
    setup_workload(cfg, data);
    WorkloadResult dotResult = run_openmp(cfg, data);
    setup_workload(cfg, data);
    WorkloadResult dotSeq = run_sequential(cfg, data);
    cfg.workload = PI;
    setup_workload(cfg, data);
    WorkloadResult piResult = run_openmp(cfg, data);
    setup_workload(cfg, data);
    WorkloadResult piSeq = run_sequential(cfg, data);

    //output
    cout << "OPENMP RESULTS -------" << endl;
    cout << "=== LOOP ===" << endl;
    cout << "Value: " << loopResult.value << endl;
    cout << "Checksum: " << loopResult.checksum << endl;
    cout << "=== DOT ===" << endl;
    cout << "Value: " << dotResult.value << endl;
    cout << "Checksum: " << dotResult.checksum << endl;
    cout << "=== PI ===" << endl;
    cout << "Value: " << piResult.value << endl;
    cout << "Checksum: " << piResult.checksum << endl;

    cout << endl;

    cout << "SEQUENTIAL RESULTS -------" << endl;
    cout << "=== LOOP ===" << endl;
    cout << "Value: " << loopSeq.value << endl;
    cout << "Checksum: " << loopSeq.checksum << endl;
    cout << "=== DOT ===" << endl;
    cout << "Value: " << dotSeq.value << endl;
    cout << "Checksum: " << dotSeq.checksum << endl;
    cout << "=== PI ===" << endl;
    cout << "Value: " << piSeq.value << endl;
    cout << "Checksum: " << piSeq.checksum << endl;
    return 0;
}
//
// Created by Chandler Radcliffe on 11/5/26.
//
#include "input.h"

// Conversions from string to enums
bool parse_workload(const string& s, WORKLOAD& w) {
    if (s == "LOOP") { w = LOOP; return true; }
    if (s == "DOT") { w = DOT; return true; }
    if (s == "PI") { w = PI; return true; }
    return false;
}

bool parse_mode(const string& s, MODE& m) {
    if (s == "SEQUENTIAL") { m = SEQUENTIAL; return true; }
    if (s == "STD_THREAD") { m = STD_THREAD; return true; }
    if (s == "OPENMP") { m = OPENMP; return true; }
    if (s == "MPI") { m = MPI; return true; }
    if (s == "COMPARE") { m = COMPARE; return true; }
    if (s == "TRACE") { m = TRACE; return true; }
    return false;
}

bool parse_model(const string& s, MODEL& m) {
    if (s == "STD_THREAD") { m = MODEL_STD_THREAD; return true; }
    if (s == "OPENMP") { m = MODEL_OPENMP; return true; }
    if (s == "MPI") { m = MODEL_MPI; return true; }
    return false;
}

bool parse_schedule(const string& s, SCHEDULE& sch) {
    if (s == "STATIC") { sch = STATIC; return true; }
    if (s == "CYCLIC") { sch = CYCLIC; return true; }
    if (s == "DYNAMIC") { sch = DYNAMIC; return true; }
    return false;
}

// Conversions from enums to string
string workload_to_string(WORKLOAD w) {
    switch (w) {
        case LOOP: return "LOOP";
        case DOT: return "DOT";
        case PI: return "PI";
    }
    return "UNKNOWN";
}

string mode_to_string(MODE m) {
    switch (m) {
        case SEQUENTIAL: return "SEQUENTIAL";
        case STD_THREAD: return "STD_THREAD";
        case OPENMP: return "OPENMP";
        case MPI: return "MPI";
        case COMPARE: return "COMPARE";
        case TRACE: return "TRACE";
    }
    return "UNKNOWN";
}

string model_to_string(MODEL m) {
    switch (m) {
        case MODEL_STD_THREAD: return "STD_THREAD";
        case MODEL_OPENMP: return "OPENMP";
        case MODEL_MPI: return "MPI";
        case MODEL_NONE: return "NONE";
    }
    return "UNKNOWN";
}

string schedule_to_string(SCHEDULE s) {
    switch (s) {
        case STATIC: return "STATIC";
        case CYCLIC: return "CYCLIC";
        case DYNAMIC: return "DYNAMIC";
    }
    return "UNKNOWN";
}

// Helper to read next field value, optionally skipping the label if present
static bool read_field_token(istream& is, const string& label, string& val, string& err) {
    string tok;
    if (!(is >> tok)) {
        err = "Unexpected end of input while expecting " + label;
        return false;
    }
    // If token matches field label (with optional colon), read the actual value
    if (tok == label || tok == label + ":") {
        if (!(is >> tok)) {
            err = "Missing value after " + label;
            return false;
        }
    }
    val = tok;
    return true;
}

/*
 * Validates parsed configuration according to specification rules
 */
bool validate_config(const input& in, string& err) {
    // 1. Workload validation
    if (in.workload_str != "LOOP" && in.workload_str != "DOT" && in.workload_str != "PI") {
        err = "Invalid WORKLOAD '" + in.workload_str + "'. Must be LOOP, DOT, or PI.";
        return false;
    }

    // 2. Mode validation
    if (in.mode_str != "SEQUENTIAL" && in.mode_str != "STD_THREAD" &&
        in.mode_str != "OPENMP" && in.mode_str != "MPI" &&
        in.mode_str != "COMPARE" && in.mode_str != "TRACE") {
        err = "Invalid MODE '" + in.mode_str + "'. Must be SEQUENTIAL, STD_THREAD, OPENMP, MPI, COMPARE, or TRACE.";
        return false;
    }

    // 3. Model validation (required ONLY when mode is TRACE)
    if (in.mode == TRACE) {
        if (in.model != MODEL_STD_THREAD && in.model != MODEL_OPENMP && in.model != MODEL_MPI) {
            err = "Invalid MODEL '" + in.model_str + "' for TRACE mode. Must be STD_THREAD, OPENMP, or MPI.";
            return false;
        }
    }

    // 4. Size validation (n > 0)
    if (in.size <= 0) {
        err = "Invalid SIZE " + to_string(in.size) + ". Must be a positive integer.";
        return false;
    }

    // 5. Threads validation
    if (in.mode == STD_THREAD || in.mode == OPENMP || in.mode == COMPARE ||
        (in.mode == TRACE && (in.model == MODEL_STD_THREAD || in.model == MODEL_OPENMP))) {
        if (in.threads <= 0) {
            err = "Invalid THREADS " + to_string(in.threads) + ". Must be at least 1.";
            return false;
        }
    }

    // 6. Schedule validation
    if (in.schedule_str != "STATIC" && in.schedule_str != "CYCLIC" && in.schedule_str != "DYNAMIC") {
        err = "Invalid SCHEDULE '" + in.schedule_str + "'. Must be STATIC, CYCLIC, or DYNAMIC.";
        return false;
    }

    // 7. Processes validation
    // MPI dynamic scheduling requires at least two ranks; comparison mode also requires at least two ranks.
    // For non-MPI modes, this field is ignored.
    bool is_mpi_active = (in.mode == MPI) || (in.mode == COMPARE) || (in.mode == TRACE && in.model == MODEL_MPI);
    if (in.mode == COMPARE) {
        if (in.processes < 2) {
            err = "COMPARE mode requires at least 2 processes (got " + to_string(in.processes) + ").";
            return false;
        }
    } else if (is_mpi_active && in.schedule == DYNAMIC) {
        if (in.processes < 2) {
            err = "MPI DYNAMIC schedule requires at least 2 processes (manager + worker, got " + to_string(in.processes) + ").";
            return false;
        }
    } else if (is_mpi_active) {
        if (in.processes < 1) {
            err = "MPI mode requires at least 1 process (got " + to_string(in.processes) + ").";
            return false;
        }
    }

    // 8. Chunk validation
    // c is a positive chunk size for cyclic or dynamic scheduling. CHUNK is ignored for static scheduling.
    if (in.schedule == CYCLIC || in.schedule == DYNAMIC) {
        if (in.chunk <= 0) {
            err = "Invalid CHUNK " + to_string(in.chunk) + ". CYCLIC and DYNAMIC schedules require CHUNK > 0.";
            return false;
        }
    }

    // 9. Repetitions validation
    if (in.repetitions <= 0) {
        err = "Invalid REPETITIONS " + to_string(in.repetitions) + ". Must be at least 1.";
        return false;
    }

    return true;
}

/*
 * Validates that requested process count equals actual MPI communicator size
 */
bool validate_mpi_comm_size(const input& in, int actual_comm_size, string& err) {
    bool is_mpi_active = (in.mode == MPI) || (in.mode == COMPARE) || (in.mode == TRACE && in.model == MODEL_MPI);
    if (is_mpi_active) {
        if (in.processes != actual_comm_size) {
            err = "Requested process count (" + to_string(in.processes) +
                  ") does not match MPI communicator size (" + to_string(actual_comm_size) + ").";
            return false;
        }
    }
    return true;
}

/*
 * Reads and parses complete configuration from an input stream
 */
bool parse_input(istream& is, input& in, string& err) {
    // 1. WORKLOAD
    cout << "WORKLOAD ";
    if (!read_field_token(is, "WORKLOAD", in.workload_str, err)) return false;
    if (!parse_workload(in.workload_str, in.workload)) {
        err = "Unknown WORKLOAD: '" + in.workload_str + "'. Expected LOOP, DOT, or PI.";
        return false;
    }

    // 2. MODE
    cout << "MODE ";
    if (!read_field_token(is, "MODE", in.mode_str, err)) return false;
    if (!parse_mode(in.mode_str, in.mode)) {
        err = "Unknown MODE: '" + in.mode_str + "'. Expected SEQUENTIAL, STD_THREAD, OPENMP, MPI, COMPARE, or TRACE.";
        return false;
    }

    // 3. MODEL (only when MODE is TRACE)
    if (in.mode == TRACE) {
        cout << "MODEL ";
        if (!read_field_token(is, "MODEL", in.model_str, err)) return false;
        if (!parse_model(in.model_str, in.model)) {
            err = "Unknown MODEL: '" + in.model_str + "'. Expected STD_THREAD, OPENMP, or MPI.";
            return false;
        }
    } else {
        in.model = MODEL_NONE;
        in.model_str = "NONE";
    }

    // 4. SIZE
    string size_str;
    cout << "SIZE ";
    if (!read_field_token(is, "SIZE", size_str, err)) return false;
    try {
        in.size = stoll(size_str);
    } catch (...) {
        err = "Invalid numeric format for SIZE: '" + size_str + "'";
        return false;
    }

    // 5. THREADS
    string threads_str;
    cout << "THREADS ";
    if (!read_field_token(is, "THREADS", threads_str, err)) return false;
    try {
        in.threads = stoi(threads_str);
    } catch (...) {
        err = "Invalid numeric format for THREADS: '" + threads_str + "'";
        return false;
    }

    // 6. PROCESSES
    string processes_str;
    cout << "PROCESSES ";
    if (!read_field_token(is, "PROCESSES", processes_str, err)) return false;
    try {
        in.processes = stoi(processes_str);
    } catch (...) {
        err = "Invalid numeric format for PROCESSES: '" + processes_str + "'";
        return false;
    }

    // 7. SCHEDULE
    cout << "SCHEDULE ";
    if (!read_field_token(is, "SCHEDULE", in.schedule_str, err)) return false;
    if (!parse_schedule(in.schedule_str, in.schedule)) {
        err = "Unknown SCHEDULE: '" + in.schedule_str + "'. Expected STATIC, CYCLIC, or DYNAMIC.";
        return false;
    }

    // 8. CHUNK
    cout << "CHUNK ";
    string chunk_str;
    if (!read_field_token(is, "CHUNK", chunk_str, err)) return false;
    try {
        in.chunk = stoi(chunk_str);
    } catch (...) {
        err = "Invalid numeric format for CHUNK: '" + chunk_str + "'";
        return false;
    }

    // 9. SEED
    string seed_str;
    cout << "SEED ";
    if (!read_field_token(is, "SEED", seed_str, err)) return false;
    try {
        in.seed = stoull(seed_str);
    } catch (...) {
        err = "Invalid numeric format for SEED: '" + seed_str + "'";
        return false;
    }

    // 10. REPETITIONS
    string repetitions_str;
    cout << "REPETITIONS ";
    if (!read_field_token(is, "REPETITIONS", repetitions_str, err)) return false;
    try {
        in.repetitions = stoi(repetitions_str);
    } catch (...) {
        err = "Invalid numeric format for REPETITIONS: '" + repetitions_str + "'";
        return false;
    }

    // Validate the complete configuration
    return validate_config(in, err);
}

// helper func
void print_config(const input& in) {
    cout << "=== Configuration ===" << endl;
    cout << "WORKLOAD:    " << workload_to_string(in.workload) << endl;
    cout << "MODE:        " << mode_to_string(in.mode) << endl;
    if (in.mode == TRACE) {
        cout << "MODEL:       " << model_to_string(in.model) << endl;
    }
    cout << "SIZE:        " << in.size << endl;
    cout << "THREADS:     " << in.threads << endl;
    cout << "PROCESSES:   " << in.processes << endl;
    cout << "SCHEDULE:    " << schedule_to_string(in.schedule) << endl;
    cout << "CHUNK:       " << in.chunk << endl;
    cout << "SEED:        " << in.seed << endl;
    cout << "REPETITIONS: " << in.repetitions << endl;
    cout << "=====================" << endl;
}
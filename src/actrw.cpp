#include"configs.h"
#include"utils.h"  
#include"module.h" 
#include<fstream>  
#include<iostream> 
#include<string.h>

using namespace std;

enum InterState {
    Activations_k = 0,
    Bias1_k,
    Empty_k
};

enum State {
    Input_k = 0,
    Output_k,
    Compute_single_k,
    Compute_all_k
};

ActRW::ActRW()
        : BaseModule() {
    bank_size = (ACTRW_maxcapacity - 1) / NUM_PE + 1;

    int memory_size = bank_size * NUM_PE * sizeof(int32_t);
    ACTmem[0] = static_cast<Memory>(new uint32_t[memory_size]);
    ACTmem[1] = static_cast<Memory>(new uint32_t[memory_size]);

    which = 0;
    internal_state = 0;
    has_bias = 0;

    read_addr_reg = 0;
    end_addr_reg = 0;
    for (int i = 0; i < NUM_PE; i++) {
        read_addr_arithm[i] = 0;
        write_addr_arithm[i] = 0;
        write_data_arithm[i] = 0;
        write_enable[i] = 0;
    }

    valid_write_times = 0;

}

ActRW::~ActRW() {
    delete[] ACTmem[0];
    delete[] ACTmem[1];
}

void ActRW::set_state(int state_t, int input_size_t, int which_t, int bias_t) {
    state = state_t;
    which = which_t;
    has_bias = bias_t;

    if (input_size_t > ACTRW_maxcapacity) {
        LOG_ERROR("End address exceeds memory capacity!");
    }
    input_size = input_size_t;
    end_addr_reg = (input_size - 1) / NUM_PE;
}

void ActRW::init() {
    string filename = datafile[ActRW_k];
    ifstream file(filename.c_str(), ios::in | ios::binary);
    int memory_size = bank_size * NUM_PE * sizeof(int32_t);
    memset(ACTmem[0], 0, memory_size);
    memset(ACTmem[1], 0, memory_size);
    if (file.is_open()) {

        if (!file.read(reinterpret_cast<char*>(ACTmem[which]), input_size * sizeof(int32_t))) {
            LOG_ERROR("File size does not match!");
        }
        file.close();
    } else {
        LOG_ERROR("Unable to open the file!");
    }
}

void ActRW::propagate() {
    int nzf_id = which;
    int arithm_id = 1 - which;
    // To Nonzero Fetch module
    if (internal_state == Activations_k) {
        for (int i = 0; i < NUM_PE; i++) {
            acts_per_bank[i] = ACTmem[nzf_id][read_addr_reg * NUM_PE + i];
        }
        reg_addr_w = read_addr_reg;
        read_addr_reg_D = read_addr_reg + 1;
        int next_state = (has_bias) ? Bias1_k : Empty_k;
        internal_state_D = (read_addr_reg == end_addr_reg) ? next_state : Activations_k;
    } else if (internal_state == Bias1_k) {
        for (int i = 1; i < NUM_PE; i++) {
            acts_per_bank[i] = 0;
        }
        *(reinterpret_cast<float*>(acts_per_bank)) = 1.0f;
        reg_addr_w = end_addr_reg + 1;
        read_addr_reg_D = 0;
        internal_state_D = Empty_k;
    } else if (internal_state == Empty_k) {
        for (int i = 0; i < NUM_PE; i++) {
            acts_per_bank[i] = 0;
        }
        reg_addr_w = 0;
        read_addr_reg_D = 0;
        internal_state_D = Empty_k;
    } else {
        LOG_ERROR("Unknown Inter State");
    }

    // To Arithmetic Modules
    write_complete = 1;
    for (int i = 0; i < NUM_PE; i++) {
        read_data_arithm[i] = ACTmem[arithm_id][read_addr_arithm[i] * NUM_PE + i];
        write_complete = write_complete && !write_enable[i];
    }

    // Control signal
    layer_complete = write_complete && (internal_state == Empty_k);
}

void ActRW::update() {
    int arithm_id = 1 - which;

    // Of Nonzero-Fetch Module
    if (*next_reg_addr) {
        read_addr_reg = read_addr_reg_D;
        internal_state = internal_state_D;
    }

    // Of Arithmetic modules
    for (int i = 0; i < NUM_PE; i++) {
        if (*(write_enable_D[i])) {
            /*
             if (*(write_addr_arithm_D[i]) * NUM_PE + i >= ACTRW_maxcapacity) {
             LOG_ERROR("error here! i:"+std::to_string(i) + ":" + std::to_string(*(write_addr_arithm_D[i])));
             }
             */
            ACTmem[arithm_id][*(write_addr_arithm_D[i]) * NUM_PE + i] = *(write_data_arithm_D[i]);
            valid_write_times++;
        }
        read_addr_arithm[i] = *(read_addr_arithm_D[i]);
        write_data_arithm[i] = *(write_data_arithm_D[i]);
        write_addr_arithm[i] = *(write_data_arithm_D[i]);
        write_enable[i] = *(write_enable_D[i]);
    }
}

void ActRW::connect(BaseModule *dependency) {
    if (dependency->name() == NzeroFetch_k) {
        NzeroFetch *module_d = static_cast<NzeroFetch*>(dependency);
        next_reg_addr = static_cast<SharedWire>(&(module_d->next_reg_addr));
    } else if (dependency->name() == Arithm_k) {
        ArithmUnit *module_d = static_cast<ArithmUnit*>(dependency);
        read_addr_arithm_D[dependency->id()] = &(module_d->read_addr);
        write_addr_arithm_D[dependency->id()] = &(module_d->write_addr);
        write_data_arithm_D[dependency->id()] = &(module_d->write_data);
        write_enable_D[dependency->id()] = &(module_d->write_enable);
    } else {
        LOG_ERROR("Unknown Module Type!");
    }

}

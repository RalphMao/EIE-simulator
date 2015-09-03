#include"configs.h"
#include"utils.h"  
#include"module.h" 
#include<fstream>  
#include<iostream> 
using namespace std;

bool ArithmUnit::initialized;
uint32_t ArithmUnit::codebook[ARITHM_codebooksize];

ArithmUnit::ArithmUnit(int id)
        : BaseModule(id) {
    codebook_size = ARITHM_codebooksize;

    valid = 0;
    index = 0;
    value_code = 0;
    act_value = 0;
    patch_complete = 0;

    valid_p = 0;
    read_addr_last = 0;

    read_addr_p = 0;
    value_decode = 0;
    act_value_p = 0;
    read_data = 0;
    result_muladd = 0;

    initialized = false;
}

void ArithmUnit::init() {
    if (!initialized) {
        string filename = datafile[Arithm_k];
        ifstream file(filename.c_str(), ios::in | ios::binary);
        if (file.is_open()) {
            int memory_size = codebook_size * sizeof(int32_t);
            if (!file.read(reinterpret_cast<char*>(codebook), memory_size)) {
                LOG_ERROR("File size does not match!");
            }
            file.close();
        } else {
            LOG_ERROR("Unable to open the file!");
        }
        initialized = true;
    }
}

void ArithmUnit::propagate() {
    // Stage 1
    read_addr = index + read_addr_last;
    read_addr_last_D = (patch_complete) ? 0 : (read_addr + 1);
    value_decode_D = codebook[value_code];
    act_value_w = act_value;
    valid_w = valid;

    // Stage 2
    bypass = valid_p_p && (read_addr_p == read_addr_p_p);
    *(reinterpret_cast<float*>(&result_mul_D)) = *(reinterpret_cast<float*>(&value_decode))
            * *(reinterpret_cast<float*>(&act_value_p));
    valid_p_w = valid_p;
    read_addr_p_w = read_addr_p;

    // Stage 3
    *(reinterpret_cast<float*>(&result_muladd)) = *(reinterpret_cast<float*>(&result_mul))
            + *(reinterpret_cast<float*>(&read_data));

    write_enable = valid_p_p;
    write_addr = read_addr_p_p;
    write_data = result_muladd;

}

void ArithmUnit::update() {
    // Stage 1
    patch_complete = *patch_complete_D;
    index = *index_D;
    value_code = *value_code_D;
    act_value = *act_value_D;
    valid = *valid_D;

    // Stage 2
    if (valid_w) {
        read_addr_last = read_addr_last_D;
    } else {
        LOG("not valid!");
    }
    read_addr_p = read_addr;
    value_decode = value_decode_D;
    act_value_p = act_value_w;
    valid_p = valid_w;

    // Stage 3
    read_addr_p_p = read_addr_p_w;
    result_mul = result_mul_D;
    valid_p_p = valid_p_w;
    if (bypass) {
        read_data = write_data;
    } else {
        read_data = *read_data_D;
    }
}

void ArithmUnit::connect(BaseModule *dependency) {
    if (dependency->name() == SpMatRead_k) {
        if (dependency->id() != module_id) {
            LOG_ERROR("Module ID does not match!");
        }
        SpMatRead *module_d = static_cast<SpMatRead*>(dependency);
        patch_complete_D = static_cast<SharedWire>(&(module_d->patch_complete));
        index_D = static_cast<SharedWire>(&(module_d->index));
        value_code_D = static_cast<SharedWire>(&(module_d->code));
        act_value_D = static_cast<SharedWire>(&(module_d->value_next));
        valid_D = static_cast<SharedWire>(&(module_d->valid_next));
    } else if (dependency->name() == ActRW_k) {
        ActRW *module_d = static_cast<ActRW*>(dependency);
        read_data_D = static_cast<SharedWire>(module_d->read_data_arithm + module_id);
    }
}


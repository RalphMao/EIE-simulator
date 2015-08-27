#include"configs.h"
#include"utils.h"  
#include"module.h" 
#include<fstream>  
#include<iostream> 

bool ArithmUnit::initialized;
uint32_t ArithmUnit::codebook[ARITHM_codebooksize];

ArithmUnit::ArithmUnit(int id) : BaseModule(id) {
    codebook_size = ARITHM_codebooksize;

    valid = 0;
    index = 0;
    value_code = 0;
    act_value = 0;
    patch_complete = 0;

    valid_p = 0;
    valid_p_p = 0;
    read_addr_last = 0;

    initialized = false;
}

void ArithmUnit::init(const char* datafile) {
    using namespace std;                                               
    if (!initialized) {
        ifstream file(datafile, ios::in|ios::binary);                      
        if (file.is_open()) {                                              
            int memory_size = codebook_size * sizeof(int32_t);
            if (!file.read(reinterpret_cast<char*>(codebook), memory_size)) { 
                LOG_ERROR("File size does not match!");                    
            }                                                              
            file.close();                                                  
        }                                                                  
        else {                                                             
            LOG_ERROR("Unable to open the file!");                         
        }                                                                  
        initialized = true;
    }
}

void ArithmUnit::propagate() {
    // Stage 1
    read_addr = index + read_addr_last;
    read_addr_last_D = (patch_complete)?0:(read_addr+1);
    valid_w = valid;
    value_decode_D = codebook[value_code];
    
    // Stage 2
    bypass = valid_p_p && (read_addr_p == read_addr_p_p);
    *(reinterpret_cast<float*>(&result_muladd)) = 
        *(reinterpret_cast<float*>(&value_decode)) * *(reinterpret_cast<float*>(&act_value_p)) +
        ((bypass)? *(reinterpret_cast<float*>(&result_muladd_p)) : *(reinterpret_cast<float*>(&read_data)));

    write_enable = valid_p;
    write_addr = read_addr_p;
    write_data = result_muladd;

    read_addr_p_w = read_addr_p;
    valid_p_w = valid_p;
    // Stage 3
    /// None

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
    }
    value_decode = value_decode_D;
    read_addr_p = read_addr;
    value_decode = value_decode_D;
    act_value_p = act_value;
    valid_p = valid_w;
    read_data = *read_data_D;

    // Stage 3
    read_addr_p_p = read_addr_p_w;
    result_muladd_p = result_muladd;
    valid_p_p = valid_p_w;
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
    }
    else if (dependency->name() == ActRW_k) {
        ActRW *module_d = static_cast<ActRW*>(dependency);
        read_data_D = static_cast<SharedWire>(module_d->read_data_arithm + module_id);
    }
}
        


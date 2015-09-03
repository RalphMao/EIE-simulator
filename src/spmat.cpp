#include"configs.h"
#include"utils.h"
#include"module.h"
#include<fstream>
#include<iostream>
using namespace std;

SpMatRead::SpMatRead(int id)
        : BaseModule(id) {
    unit_line = SPMAT_unit_line;
    num_lines = SPMAT_num_lines;
    index_bits = SPMAT_index_bits;
    weights_bits = SPMAT_weights_bits;

    int memory_size = unit_line * num_lines * sizeof(int32_t) * 2;
    WImem = static_cast<Memory>(new uint32_t[memory_size]);

    memory_addr = 0;
    read_enable = 0;
    memory_shift = 0;
    patch_complete = 1;
    valid = 0;
    value = 0;

}

SpMatRead::~SpMatRead() {
    delete[] WImem;
}

void SpMatRead::init() {
    string filename = datafile[SpMatRead_k];
    filename += to_string(id());
    filename += ".dat";
    ifstream file(filename.c_str(), ios::in | ios::binary);
    ifstream file_test(filename.c_str(), ios::in | ios::binary | ios::ate);
    if (file.is_open()) {
        int memory_size = unit_line * num_lines * sizeof(int32_t) * 2;
        int file_size = static_cast<int>(file_test.tellg());

        if (file_size > memory_size) {
            LOG_ERROR("File size exceeds memory limit!");
        }

        if (!file.read(reinterpret_cast<char*>(WImem), file_size)) {
            LOG_ERROR("File size does not match!");
        }
        for (int i = 0; i < 2 * unit_line; i++) {
            data_read[i] = WImem[i];
        }

        file.close();
    } else {
        LOG_ERROR("Unable to open the file!");
    }
}

void SpMatRead::propagate() {

    // Memory access
    if (read_enable) {
        for (int i = 0; i < 2 * unit_line; i++) {
            data_read[i] = WImem[memory_addr * unit_line * 2 + i];
        }
    }
    code = data_read[memory_shift * 2];
    index = data_read[memory_shift * 2 + 1];

    value_w = value;
    valid_w = valid;
    patch_complete_w = patch_complete;

}

void SpMatRead::update() {
    if (*valid_D) {
        patch_complete = *patch_complete_D;
        memory_shift = *memory_shift_D;
        memory_addr = *memory_addr_D;
        value = *value_D;
    }
    read_enable = *read_enable_D;
    valid = *valid_D;
}

void SpMatRead::connect(BaseModule *dependency) {
    if (dependency->id() != module_id) {
        LOG_ERROR("Module ID does not match!");
    }

    if (dependency->name() == PtrRead_k) {
        PtrRead *module_d = static_cast<PtrRead*>(dependency);

        patch_complete_D = static_cast<SharedWire>(&(module_d->patch_complete));
        memory_shift_D = static_cast<SharedWire>(&(module_d->memory_shift));
        memory_addr_D = static_cast<SharedWire>(&(module_d->memory_addr));
        read_enable_D = static_cast<SharedWire>(&(module_d->read_spmat));
        valid_D = static_cast<SharedWire>(&(module_d->valid));
        value_D = static_cast<SharedWire>(&(module_d->value_w));
    } else {
        LOG_ERROR("Unknown Module Type!");
    }
}


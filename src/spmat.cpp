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

    start_addr = 0;
    end_addr = 0;
    valid = 0;
    value = 0;

    start_addr_nextline = 0;
    current_addr_shift = 0;
    patch_complete_p = 1;
    line_complete_p = 1;
    memory_addr_shift_p = 0;
    for (int i = 0; i < 2 * unit_line; i++) {
        data_read[i] = 0;
    }

}

SpMatRead::~SpMatRead() {
    delete[] WImem;
}

void SpMatRead::init() {
    string filename = datafile[SpMatRead_k];
    filename += to_string(id());
    filename += ".dat";
    ifstream file(filename.c_str(), ios::in | ios::binary);
    ifstream file_test(filename, ios::in | ios::binary | ios::ate);
    if (file.is_open()) {
        int memory_size = unit_line * num_lines * sizeof(int32_t) * 2;
        int file_size = static_cast<int>(file_test.tellg());

        if (file_size > memory_size) {
            LOG_ERROR("File size exceeds memory limit!");
        }

        if (!file.read(reinterpret_cast<char*>(WImem), file_size)) {
            LOG_ERROR("File size does not match!");
        }
        file.close();
    } else {
        LOG_ERROR("Unable to open the file!");
    }
}

void SpMatRead::connect(BaseModule *dependency) {
    if (dependency->id() != module_id) {
        LOG_ERROR("Module ID does not match!");
    }

    if (dependency->name() == PtrRead_k) {
        PtrRead *module_d = static_cast<PtrRead*>(dependency);

        start_addr_D = static_cast<SharedWire>(&(module_d->start_addr));
        end_addr_D = static_cast<SharedWire>(&(module_d->end_addr));
        valid_D = static_cast<SharedWire>(&(module_d->valid));
        value_D = static_cast<SharedWire>(&(module_d->value_w));
    } else {
        LOG_ERROR("Unknown Module Type!");
    }
}

void SpMatRead::propagate() {
    addr = (patch_complete_p) ? start_addr : start_addr_nextline * unit_line;
    memory_addr = addr / unit_line;
    addr_residue = addr % unit_line;
    memory_shift = (line_complete_p) ? addr_residue : current_addr_shift;
    memory_addr_shift = memory_addr * unit_line + memory_shift;

    start_addr_nextline_D = memory_addr + 1;
    current_addr_shift_D = memory_shift + 1;

    // Memory access
    if (valid && line_complete_p) {
        for (int i = 0; i < 2 * unit_line; i++) {
            data_read[i] = WImem[memory_addr * unit_line * 2 + i];
        }
    }
    code = data_read[memory_shift * 2];
    index = data_read[memory_shift * 2 + 1];

    value_next = value;

    // valid_next = valid && (memory_addr_shift_p != memory_addr_shift); 
    valid_next = valid;

    // Decision
    line_last = memory_addr == (end_addr / unit_line);
    shift_equal = memory_shift == (end_addr % unit_line);
    line_complete = (line_last) ? shift_equal : ((memory_shift + 1) == static_cast<uint32_t>(unit_line));
    patch_complete = line_last && shift_equal;

    read = patch_complete || !valid;
}

void SpMatRead::update() {
    if (valid) {
        line_complete_p = line_complete;
        current_addr_shift = current_addr_shift_D;
        memory_addr_shift_p = memory_addr_shift;
        if (line_complete) {
            start_addr_nextline = start_addr_nextline_D;
            patch_complete_p = patch_complete;
        }
    }

    if (read) {
        start_addr = *start_addr_D;
        end_addr = *end_addr_D;
        valid = *valid_D;
        value = *value_D;
    }
}


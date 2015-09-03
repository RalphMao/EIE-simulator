#include"configs.h"
#include"utils.h"
#include"module.h"
#include<fstream>
#include<iostream>

PtrRead::PtrRead(int id)
        : BaseModule(id) {
    num_lines = PTRVEC_num_lines;
    unit_line = SPMAT_unit_line;

    int memory_size = num_lines * sizeof(int32_t);
    PTRmem = static_cast<Memory>(new uint32_t[memory_size]);

    ptr_odd_addr = 0;
    ptr_even_addr = 0;
    index_flag = 0;
    value = 0;
    empty = 1;

    start_addr_p = 0;
    memory_addr_p = 0;
    patch_complete_p = 1;
}

PtrRead::~PtrRead() {
    delete[] PTRmem;
}
void PtrRead::init(const char *datafile) {
    using namespace std;
    ifstream file(datafile, ios::in | ios::binary);
    if (file.is_open()) {
        int memory_size = num_lines * sizeof(int32_t);

        if (!file.read(reinterpret_cast<char*>(PTRmem), memory_size)) {
            LOG_ERROR("File size does not match!");
        }
        file.close();
    } else {
        LOG_ERROR("Unable to open the file!");
    }

}

void PtrRead::propagate() {
    index_odd = PTRmem[ptr_odd_addr * 2 + 1];
    index_even = PTRmem[ptr_even_addr * 2];
    if (index_flag) {
        start_addr = index_odd;
        end_addr = index_even - 1;
    } else {
        start_addr = index_even;
        end_addr = index_odd - 1;
    }

    valid = !((index_even == index_odd) || (empty));
    value_w = value;

    // New
    current_addr = patch_complete_p ? start_addr : start_addr_p;
    memory_addr = current_addr / unit_line;
    memory_shift = current_addr % unit_line;
    read_spmat = (memory_addr != memory_addr_p) && valid;

    read_ptr = !valid || patch_complete;

}

void PtrRead::update() {
    if (read_ptr) {
        if (!*empty_D) {
            ptr_odd_addr = *ptr_odd_addr_D;
            ptr_even_addr = *ptr_even_addr_D;
            index_flag = *index_flag_D;
            value = *value_D;
        }
        empty = *empty_D;
    }
}

void PtrRead::connect(BaseModule *dependency) {
    if (dependency->name() == NzeroFetch_k) {
        NzeroFetch *module_d = static_cast<NzeroFetch*>(dependency);
        ptr_odd_addr_D = static_cast<SharedWire>(module_d->ptr_odd_addr + this->id());
        ptr_even_addr_D = static_cast<SharedWire>(module_d->ptr_even_addr + this->id());
        index_flag_D = static_cast<SharedWire>(module_d->index_flag + this->id());
        value_D = static_cast<SharedWire>(module_d->value_output + this->id());
        empty_D = static_cast<SharedWire>(module_d->empty + this->id());
    } else {
        LOG_ERROR("Unknown Module Type!");
    }
}


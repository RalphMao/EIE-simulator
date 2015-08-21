#include"configs.h"
#include"utils.h"
#include"module.h"
#include<fstream>
#include<iostream>

PtrRead::PtrRead(int id) :BaseModule(id) {
    num_lines = PTRVEC_num_lines;

    int memory_size = num_lines * sizeof(int32_t);
    PTRmem = static_cast<Memory>(new uint32_t[memory_size]);

    act_index = 0;
    value = 0;
    empty = 1;
}

PtrRead::~PtrRead(){
    delete[] PTRmem;
}
void PtrRead::init(const char *datafile) {
    using namespace std;
    ifstream file(datafile, ios::in|ios::binary);
    if (file.is_open()) {
        int memory_size = num_lines * sizeof(int32_t);

        if (!file.read(reinterpret_cast<char*>(PTRmem), memory_size)) {
            LOG_ERROR("File size does not match!");
        }
        file.close();
    }
    else {
        LOG_ERROR("Unable to open the file!");
    }

}

void PtrRead::propagate() {
    index_odd = PTRmem[act_index | 0x00000001];
    index_even = PTRmem[(act_index + 1) & 0xfffffffe];
    if (act_index & 0x00000001) {
        start_addr = index_odd;
        end_addr = index_even - 1;
    }
    else {
        start_addr = index_even;
        end_addr = index_odd - 1;
    }

    valid = !((index_even == index_odd) || (empty));
}

void PtrRead::update() {
    if (!valid ||(*read_sp)) {
        act_index = *act_index_D;
        value = *value_D;
        empty = *empty_D;
    }
}

void PtrRead::connect(BaseModule *dependency) {
    if (dependency->name() == SpMatRead_k) {
        if (dependency->id() != module_id) {
             LOG_ERROR("Module ID does not match!");
        }
        SpMatRead* module_d = static_cast<SpMatRead*>(dependency);
        read_sp = static_cast<SharedWire>(&(module_d->read));
        
    }
    else if (dependency->name() == NzeroFetch_k) {
        NzeroFetch *module_d = static_cast<NzeroFetch*>(dependency);
        act_index_D = static_cast<SharedWire>(module_d->act_index_output + this->id());
        value_D = static_cast<SharedWire>(module_d->value_output + this->id());
        empty_D= static_cast<SharedWire>(module_d->empty + this->id());
    }
    else {
        LOG_ERROR("Unknown Module Type!");
    }
}


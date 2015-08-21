#include"configs.h"
#include"utils.h"
#include"module.h"
#include<iostream>

#include<cstring>
using namespace std;

NzeroFetch::NzeroFetch() : BaseModule() {
    // Set parameters
    buffer_size = NZFETCH_buffersize;

    // Initialization
    for (int i = 0; i < NUM_PE; i++) {
        pos_read[i] = 0;
        pos_write[i] = 0;

        memset(act_index[i], 0, buffer_size * sizeof(Register));
        memset(value[i], 0, buffer_size * sizeof(Register));

    }

    pack_addr_p = 0;
    reg_addr = 0;

    memset(acts_per_bank, 0, NUM_PE * sizeof(Register));
}

void NzeroFetch::propagate() {

    one_full = 0;
    for (int i = 0; i < NUM_PE; i++) {
        pos_read_D[i] = (pos_read[i] + 1) % buffer_size;
        pos_write_D[i] = (pos_write[i] + 1) % buffer_size;

        full[i] = (pos_write_D[i] == pos_read[i]);
        empty[i] = (pos_write[i] == pos_read[i]);
        one_full = one_full || full[i];

        value_output[i] = value[i][pos_read[i]];
        act_index_output[i] = act_index[i][pos_read[i]];

#if DEBUG == 1
        if (full[i]) full_idx = i;
#endif
    }

    // Shortcut for arithmetic logics
    find = 0;
    for (pack_addr = (pack_addr_p + 1) % NUM_PE; pack_addr < NUM_PE; pack_addr++) {
        if (*(reinterpret_cast<float*>(acts_per_bank + pack_addr)) > 0.0f) {
            find = 1;
            break;
        }
    }

    pack_addr = (find)?pack_addr : (NUM_PE - 1);
    value_buffer = acts_per_bank[pack_addr];
    index_buffer = reg_addr * NUM_PE + pack_addr;

    // Control logics
    next_shift = !(one_full && find);
    next_reg_addr = !find || ((pack_addr == NUM_PE - 1) && !one_full);
    write_enable = find && !one_full;
}

void NzeroFetch::update() {
    if (next_shift) {
        pack_addr_p = pack_addr;
    }

    if (next_reg_addr) {
        reg_addr = *reg_addr_D;
        for (int i = 0; i < NUM_PE; i++) {
            acts_per_bank[i] = *(acts_per_bank_D[i]);
        }
    }

    if (write_enable) {
        for (int i = 0; i < NUM_PE; i++) {
            act_index[i][pos_write[i]] = index_buffer;
            value[i][pos_write[i]] = value_buffer;
            pos_write[i] = pos_write_D[i];
        }
    }

    for (int i = 0; i < NUM_PE; i++) {
        if (!empty[i] && (!*(valid_ptr[i]) || *(read_sp[i]))) {
            pos_read[i] = pos_read_D[i];
        }
    }
}

void NzeroFetch::connect(BaseModule *dependency) {
    if (dependency->name() == ActRW_k) {
        ActRW* module_d = static_cast<ActRW*>(dependency);
        reg_addr_D = static_cast<SharedWire>(&(module_d->reg_addr_w));
        for (int i=0; i < NUM_PE; i++) {
            acts_per_bank_D[i] = static_cast<SharedWire>(module_d->acts_per_bank + i);
        }
    }
    else if (dependency->name() == PtrRead_k) {
        PtrRead* module_d = static_cast<PtrRead*>(dependency);
        valid_ptr[module_d->id()] = static_cast<SharedWire>(&(module_d->valid));
    }
    if (dependency->name() == SpMatRead_k) {
        SpMatRead* module_d = static_cast<SpMatRead*>(dependency);
        read_sp[module_d->id()] = static_cast<SharedWire>(&(module_d->read));
    }
}

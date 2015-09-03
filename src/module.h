#ifndef MODULE
#define MODULE

#include<stdint.h>
#include"configs.h"

typedef uint32_t Register;
typedef uint32_t Wire;
typedef uint32_t* Memory;
typedef const uint32_t* SharedWire;

class BaseModule {
 public:

    BaseModule() {
        module_id = 0;
    }
    BaseModule(int id) {
        module_id = id;
    }
    virtual ~BaseModule() {
    }
    virtual void propagate() = 0;
    virtual void update() = 0;
    virtual void connect(BaseModule *dependency) = 0;
    virtual inline ModuleType name() {
        return Base_k;
    }
    inline int id() {
        return module_id;
    }
    int module_id;
};

class ActRW : public BaseModule {
 public:

    ActRW();
    ~ActRW();
    void init(const char* datafile);
    void set_state(int state_t, int end_addr_t, int which_t, int bias_t);
    virtual void propagate();
    virtual void update();
    virtual void connect(BaseModule *dependency);
    virtual inline ModuleType name() {
        return ActRW_k;
    }

    Memory ACTmem[2];

    // To NzeroFetch
    Register read_addr_reg, end_addr_reg;
    Register which, internal_state, has_bias;
    Register state;  // Not used so far
    Wire reg_addr_w;
    Wire read_addr_reg_D, internal_state_D;
    Wire acts_per_bank[NUM_PE];
    SharedWire next_reg_addr;

    // To Arithmetic module
    Register read_addr_arithm[NUM_PE], write_addr_arithm[NUM_PE], write_data_arithm[NUM_PE], write_enable[NUM_PE];
    Wire read_data_arithm[NUM_PE];
    Wire write_complete, layer_complete;
    SharedWire read_addr_arithm_D[NUM_PE], write_addr_arithm_D[NUM_PE], write_data_arithm_D[NUM_PE],
            write_enable_D[NUM_PE];

    int bank_size;
    int input_size;

    unsigned int valid_write_times;  // Statistics

};

class NzeroFetch : public BaseModule {
 public:
    NzeroFetch();
    virtual ~NzeroFetch() {
    }
    virtual void propagate();
    virtual void update();
    virtual void connect(BaseModule *dependency);
    virtual inline ModuleType name() {
        return NzeroFetch_k;
    }

    // Nonzero find
    Register pack_addr_p;
    Register reg_addr, acts_per_bank[NUM_PE];
    Wire one_full, find, write_enable;
    Wire pack_addr, next_shift, next_reg_addr;
    Wire value_buffer, index_buffer;
    SharedWire reg_addr_D, acts_per_bank_D[NUM_PE];

    // FIFO part
    Register pos_read[NUM_PE], pos_write[NUM_PE];
    Register act_index[NUM_PE][NZFETCH_buffersize], value[NUM_PE][NZFETCH_buffersize];
    Wire act_index_output[NUM_PE], value_output[NUM_PE];
    Wire ptr_odd_addr[NUM_PE], ptr_even_addr[NUM_PE], index_flag[NUM_PE];
    Wire empty[NUM_PE], full[NUM_PE];
    Wire pos_read_D[NUM_PE], pos_write_D[NUM_PE];
    SharedWire read_ptr[NUM_PE];

    int buffer_size;
#if DEBUG == 1
    int full_idx;
#endif

};

class PtrRead : public BaseModule {
 public:
    PtrRead(int id);
    virtual ~PtrRead();
    void init(const char *datafile);
    virtual inline ModuleType name() {
        return PtrRead_k;
    }
    virtual void propagate();
    virtual void update();
    virtual void connect(BaseModule *dependency);

    Register ptr_odd_addr, ptr_even_addr, index_flag, value, empty;
    Wire start_addr, end_addr, valid, value_w;
    Wire index_odd, index_even;
    // New
    Register start_addr_p, memory_addr_p, patch_complete_p;
    Wire patch_complete, read_ptr, read_spmat;
    Wire current_addr, memory_addr, memory_shift;
    SharedWire ptr_odd_addr_D, ptr_even_addr_D, index_flag_D, value_D, empty_D;

    Memory PTRmem;

    int num_lines, unit_line;
};

class SpMatRead : public BaseModule {
 public:
    SpMatRead(int id);
    virtual ~SpMatRead();
    void init(const char *datafile);
    virtual void propagate();
    virtual void update();
    virtual void connect(BaseModule *dependency);
    virtual inline ModuleType name() {
        return SpMatRead_k;
    }

    Register memory_addr, read_enable, memory_shift, patch_complete, valid, value;

    Wire index, code, valid_w, value_w, patch_complete_w;
    Wire data_read[SPMAT_unit_line * 2];

    SharedWire memory_addr_D, read_enable_D, memory_shift_D, patch_complete_D, valid_D, value_D;

    Memory WImem;

    int unit_line, num_lines;
    int index_bits, weights_bits;

};

class ArithmUnit : public BaseModule {
 public:
    ArithmUnit(int id);
    virtual ~ArithmUnit() {
    }
    void init(const char *datafile);
    virtual void propagate();
    virtual void update();
    virtual void connect(BaseModule *dependency);
    virtual inline ModuleType name() {
        return Arithm_k;
    }

    Register patch_complete, index, value_code, act_value, valid;
    Register read_addr_last, result_mul;
    Register read_addr_p, value_decode, act_value_p, valid_p, read_data;
    Register read_addr_p_p, valid_p_p;

    Wire read_addr, value_decode_D, read_addr_last_D;
    Wire value_to_add, result_muladd, result_mul_D;
    Wire bypass;
    Wire write_enable, write_addr, write_data;
    Wire valid_w, valid_p_w, read_addr_p_w, act_value_w;

    SharedWire patch_complete_D, index_D, value_code_D, act_value_D, valid_D;
    SharedWire read_data_D;

    static uint32_t codebook[ARITHM_codebooksize];
    static bool initialized;
    int codebook_size;
};

#endif

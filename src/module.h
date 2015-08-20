
#ifndef MODULE
#define MODULE

#include<stdint.h>
#include"configs.h"

typedef int32_t Register;
typedef int32_t Wire;
typedef int32_t* Memory;
typedef const int32_t* SharedWire;

enum ModuleType {
    ActRW_k = 0,
    NzeroFetch_k,
    PtrRead_k,
    SpMatRead_k,
    Arithm_k,
    // WDecode_k,
    // AddMulUnit_k,
    Control_k,
    Base_k
};
const int TYPES = Base_k;

enum ConnectType {
    NoConnect = 0,
    Connect_by_id,
    Connect_all
};

class BaseModule {
    public:
    
    BaseModule() {module_id=0;}
    BaseModule(int id) {module_id=id;}
    virtual ~BaseModule() {}
    virtual void propagate() = 0;
    virtual void update() = 0;
    virtual void connect(BaseModule *dependency) = 0;
    virtual inline ModuleType name() { return Base_k;}
    inline int id() { return module_id;}
    int module_id;
};

class ActRW : public BaseModule {
    public:

    ActRW();
    ~ActRW();
    void init(char* datafile);
    virtual void propagate();
    virtual void update();
    virtual void connect(BaseModule *dependency);
    virtual inline ModuleType name() { return ActRW_k;}

    Register reg_addr;
    Wire reg_addr_w;
    Wire acts_per_bank[NUM_PE];
};


class NzeroFetch : public BaseModule {
    public:
    NzeroFetch();
    virtual ~NzeroFetch() {}
    virtual void propagate();
    virtual void update();
    virtual void connect(BaseModule *dependency);
    virtual inline ModuleType name() { return NzeroFetch_k;}

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
    Wire empty[NUM_PE], full[NUM_PE];
    Wire pos_read_D[NUM_PE], pos_write_D[NUM_PE];
    SharedWire read_sp[NUM_PE], valid_ptr[NUM_PE];

    int buffer_size;

};

class SpMatRead : public BaseModule {
    public:
    SpMatRead(int id);
    virtual ~SpMatRead();
    void init(char *datafile);
    virtual void propagate();
    virtual void update();
    virtual void connect(BaseModule *dependency);
    virtual inline ModuleType name() { return SpMatRead_k;}

    Register start_addr, end_addr, valid, value;
    Register start_addr_nextline, current_addr_shift;
    Register patch_complete_p, line_complete_p;
    Register memory_addr_shift_p;

    Wire index, code, valid_next;
    Wire start_addr_nextline_D, current_addr_shift_D;
    Wire patch_complete, line_complete, line_last, shift_equal;
    Wire memory_addr, memory_shift;
    Wire addr, addr_residue;
    Wire memory_addr_shift;
    Wire *data_read;
    Wire read;

    SharedWire start_addr_D, end_addr_D, valid_D, value_D;

    Memory WImem;

    int unit_line, num_lines;
    int index_bits, weights_bits;

};

class PtrRead : public BaseModule {
    public:
    PtrRead(int id); 
    virtual ~PtrRead();
    void init(char *datafile);
    virtual inline ModuleType name() { return PtrRead_k;}
    virtual void propagate();
    virtual void update();
    virtual void connect(BaseModule *dependency);

    Register act_index, value, empty;
    Wire start_addr, end_addr, valid;
    Wire index_odd, index_even;
    SharedWire act_index_D, value_D, empty_D, read_sp;

    Memory PTRmem;
    
    int num_lines;
};

#endif

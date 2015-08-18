
#ifndef MODULE
#define MODULE

#include<stdint.h>

typedef int32_t Register;
typedef int32_t Wire;
typedef int32_t* Memory;
typedef int32_t* SharedWire;

enum ModuleType {
    Base_k,
    ActRW_k,
    NzeroFetch_k,
    PtrRead_k,
    SpMatRead_k,
    IndAccumulate_k,
    WDecode_k,
    AddMulUnit_k,
    Control_k
};

class BaseModule {
    public:
    
    BaseModule() {module_id=0;}
    BaseModule(int id) {module_id=id;}
    virtual ~BaseModule() {}
    virtual float propagate() = 0;
    virtual float update() = 0;
    virtual void connect(BaseModule *dependency) = 0;
    virtual inline ModuleType name() { return Base_k;}
    inline int id() { return module_id;}
    int module_id;
};

class SpMatRead : public BaseModule {
    public:
    SpMatRead(int id);
    virtual ~SpMatRead();
    void init(char *datafile);
    virtual float propagate();
    virtual float update();
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


    SharedWire start_addr_D, end_addr_D, valid_D, value_D;

    Memory WImem;

    int unit_line, num_lines;
    int index_bits, weights_bits;

};

class PtrRead : public BaseModule {
    public:
    PtrRead(int id); 
    virtual ~PtrRead();
    virtual inline ModuleType name() { return PtrRead_k;}
    virtual float propagate();
    virtual float update();
    virtual void connect(BaseModule *dependency);

    Wire start_addr, end_addr, valid, value;
};

#endif

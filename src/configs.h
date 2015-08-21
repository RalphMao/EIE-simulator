
#ifndef CONFIGS
#define CONFIGS

#define DEBUG 1

const int NUM_PE = 4;

const int ACTRW_maxcapacity = 16;

const int NZFETCH_buffersize = 2;

const int PTRVEC_num_lines = 10;

const int SPMAT_unit_line   =  8;
const int SPMAT_num_lines   =  25;
const int SPMAT_index_bits  =  4;
const int SPMAT_weights_bits=  4;

const int ARITHM_codebooksize = 16;

enum ModuleType {
    ActRW_k = 0,
    NzeroFetch_k,
    PtrRead_k,
    SpMatRead_k,
    Arithm_k,
    // WDecode_k,
    // AddMulUnit_k,
    // Control_k,
    Base_k
};

const int TYPES = Base_k;
const int NumModules[TYPES] = {
    1,
    1,
    NUM_PE,
    NUM_PE,
    NUM_PE 
};

enum ConnectType {
    NoConnect = 0,
    Connect_by_id,
    Connect_all
};

const int topology[TYPES][TYPES] = {
    {0, 1, 0, 0, 1},
    {1, 0, 2, 2, 0},
    {0, 1, 0, 1, 0},
    {0, 0, 1, 0, 0},
    {1, 0, 0, 1, 0},
};

#define MAX_FILELENGTH 20
const char datafile[TYPES][MAX_FILELENGTH] = {
    "act",
    "",
    "ptr/ptr",
    "spm/spm",
    "arithm",
};

#endif

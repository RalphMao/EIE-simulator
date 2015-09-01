
#ifndef CONFIGS
#define CONFIGS

#include"params.h"

enum ModuleType {
    ActRW_k = 0,
    NzeroFetch_k,
    PtrRead_k,
    SpMatRead_k,
    Arithm_k,
    Base_k
};

const int TYPES = Base_k; //number of types: equal to the last element's number
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
    {0, 1, 0, 0, 2},
    {1, 0, 2, 2, 0},
    {0, 2, 0, 1, 0},
    {0, 0, 1, 0, 0},
    {2, 0, 0, 1, 0},
};

#define MAX_FILELENGTH 20
const char datafile[TYPES][MAX_FILELENGTH] = {
    "data/act.dat",
    "",
    "data/ptr/ptr",
    "data/spm/spm",
    "data/arithm.dat",
};

#endif

#include"module.h"
#include"utils.h"
#include<vector>
#include<iostream>

using namespace std;

PtrRead::PtrRead(int id) :BaseModule(id) {}

PtrRead::~PtrRead(){}

void init(char *datafile) {}

void PtrRead::propagate(){start_addr = 2; end_addr = 20;valid=1;}

void PtrRead::update() {}

void PtrRead::connect(BaseModule *dependency) {}

int main() {
    PtrRead PtrR_M(0);
    SpMatRead SPMat_M(0);

    SPMat_M.init("test_data/spmatrix.dat");
    SPMat_M.connect(static_cast<BaseModule*>(&PtrR_M));
    vector<BaseModule*> modules;
    modules.push_back(static_cast<BaseModule*>(&PtrR_M));
    modules.push_back(static_cast<BaseModule*>(&SPMat_M));

    int cycles = 35;
    for (int i = 0; i < cycles; i++) {
        modules[1]->propagate();
        if (i==0) {
            PtrR_M.propagate();
        }
        else if (i==19) {
            PtrR_M.start_addr = 22;
            PtrR_M.end_addr = 29;
        }
        else if (i>23 && i <= 30) {
            PtrR_M.start_addr = 30;
            PtrR_M.end_addr = 29;
            PtrR_M.valid = 0;
        }
        else if (i == 31) {
            PtrR_M.start_addr = 38;
            PtrR_M.end_addr = 38;
            PtrR_M.valid = 1;
        }

        std::cout << "Cycles:" << i << std::endl;
        P_V(SPMat_M.start_addr);
        P_V(SPMat_M.end_addr);
        P_V(SPMat_M.index);
        P_V(SPMat_M.code);
        P_V(SPMat_M.valid);
        P_V(SPMat_M.valid_next);
        P_V(SPMat_M.memory_addr);
        P_V(SPMat_M.memory_shift);
        P_V(SPMat_M.line_complete);
        P_V(SPMat_M.patch_complete);
        std::cout << "==========================================="<<std::endl;

        for (int m = 0; m < modules.size(); m++) {
            modules[m]->update();
        }

    }
}
        
        


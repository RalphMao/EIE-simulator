#include"module.h"
#include"utils.h"
#include<vector>
#include<iostream>

using namespace std;

NzeroFetch::NzeroFetch() :BaseModule() {}
void NzeroFetch::propagate() {
    act_index_output[0] = 3;
    value_output[0] = 2;
    empty[0] = 1;
}
void NzeroFetch::update() {}
void NzeroFetch::connect(BaseModule *dependency) {}


int main() {
    PtrRead PtrR_M(0);
    SpMatRead SPMat_M(0);
    NzeroFetch NZF_M;

    SPMat_M.init("test_data/spmatrix.dat");
    PtrR_M.init("test_data/ptrvec.dat");

    SPMat_M.connect(static_cast<BaseModule*>(&PtrR_M));
    PtrR_M.connect(static_cast<BaseModule*>(&NZF_M));
    PtrR_M.connect(static_cast<BaseModule*>(&SPMat_M));

    vector<BaseModule*> modules;
    modules.push_back(static_cast<BaseModule*>(&PtrR_M));
    modules.push_back(static_cast<BaseModule*>(&SPMat_M));
    modules.push_back(static_cast<BaseModule*>(&NZF_M));

    int cycles = 40;
    modules[2]->propagate();
    for (int i = 0; i < cycles; i++) {
        modules[1]->propagate();
        modules[0]->propagate();

        if (i==2) {
            NZF_M.empty[0] = 0;
        }
        else if (i == 3) {
            NZF_M.act_index_output[0] = 5;
        }
        else if (i == 4) {
            NZF_M.empty[0] = 1;
        }
        else if (i == 37) {
            NZF_M.empty[0] = 0;
            NZF_M.act_index_output[0] = 7;
        }

        std::cout << "Cycles:" << i << std::endl;
        P_V(NZF_M.empty[0]);

        P_V(PtrR_M.act_index);
        P_V(PtrR_M.empty);
        P_V(PtrR_M.index_odd);
        P_V(PtrR_M.start_addr);
        P_V(PtrR_M.valid);

        P_V(SPMat_M.start_addr);
        P_V(SPMat_M.end_addr);
        P_V(SPMat_M.index);
        P_V(SPMat_M.memory_addr_shift);
        P_V(SPMat_M.valid);
        P_V(SPMat_M.valid_next);
        P_V(SPMat_M.patch_complete);
        std::cout << "==========================================="<<std::endl;

        for (int m = 0; m < modules.size(); m++) {
            modules[m]->update();
        }

    }
}

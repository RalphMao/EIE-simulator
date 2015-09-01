#include"module.h"
#include"utils.h"
#include<vector>
#include<iostream>

using namespace std;

ActRW::ActRW()
        : BaseModule() {
}
ActRW::~ActRW() {
}
void ActRW::propagate() {
}
void ActRW::update() {
}
void ActRW::connect(BaseModule *dependency) {
}

SpMatRead::SpMatRead(int id)
        : BaseModule(id) {
}
SpMatRead::~SpMatRead() {
}
void SpMatRead::propagate() {
}
void SpMatRead::update() {
}
void SpMatRead::connect(BaseModule *dependency) {
}

int main() {
    ActRW act;
    SpMatRead spm(0);
    ArithmUnit aru(0);

    aru.init("test_data/aru.dat");

    aru.connect(static_cast<BaseModule*>(&spm));
    aru.connect(static_cast<BaseModule*>(&act));

    int cycles = 20;
    P_V(aru.initialized);
    P_V(aru.codebook[0]);
    P_V(aru.codebook[1]);
    P_V(aru.codebook[2]);

    for (int i = 0; i < cycles; i++) {
        aru.propagate();

        *(reinterpret_cast<float*>(&(act.read_data_arithm))) = (float) aru.read_addr;

        if (i == 0) {
            spm.patch_complete = 1;
            spm.index = 3;
            spm.code = 1;
            spm.valid_next = 0;
            *(reinterpret_cast<float*>(&(spm.value_next))) = 1.0;
        } else if (i == 2) {
            spm.valid_next = 1;
        } else if (i == 3) {
            spm.patch_complete = 0;
            spm.index = 1;
            spm.code = i - 1;
        } else if (i == 10) {
            spm.patch_complete = 1;
        } else if (i == 11) {
            spm.patch_complete = 0;
            spm.index = 8;
        } else if (i == 12) {
            spm.valid_next = 0;
        }
        std::cout << "Cycles:" << i << std::endl;
        P_V(aru.read_addr);
        P_V(aru.read_addr_last);
        P_V(aru.read_addr_p);
        P_V(aru.read_addr_p_p);
        P_V(*(reinterpret_cast<float*>(&(aru.read_data))));
        P_V(aru.patch_complete);
        P_V(aru.valid);
        P_V(aru.valid_p);
        P_V(aru.valid_p_p);
        P_V(aru.bypass);
        P_V(*(reinterpret_cast<float*>(&(aru.value_decode_D))));
        P_V(*(reinterpret_cast<float*>(&(aru.value_decode))));
        P_V(*(reinterpret_cast<float*>(&(aru.act_value_p))));
        P_V(*(reinterpret_cast<float*>(&(aru.write_data))));
        std::cout << "===========================================" << std::endl;

        aru.update();
    }
}


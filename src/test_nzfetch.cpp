#include"module.h"
#include"utils.h"
#include<vector>
#include<iostream>

using namespace std;

ActRW::ActRW() :BaseModule() {}
ActRW::~ActRW() {}
void ActRW::propagate() {}
void ActRW::update() {}
void ActRW::connect(BaseModule *dependency) {}

int main() {
    NzeroFetch Nzf_M;
    vector<BaseModule*> modules;
    modules.push_back(static_cast<BaseModule*>(&Nzf_M));

    for (int i=0; i<NUM_PE; i++) {
        PtrRead *ptr_M = new PtrRead(i);
        SpMatRead *spm_M = new SpMatRead(i);

        spm_M->init("test_data/spmatrix.dat");
        ptr_M->init("test_data/ptrvec.dat");

        ptr_M->connect(static_cast<BaseModule*>(spm_M));
        ptr_M->connect(static_cast<BaseModule*>(&Nzf_M));
        spm_M->connect(static_cast<BaseModule*>(ptr_M));
        spm_M->connect(static_cast<BaseModule*>(&Nzf_M));

        Nzf_M.connect(static_cast<BaseModule*>(ptr_M));
        Nzf_M.connect(static_cast<BaseModule*>(spm_M));

        modules.push_back(static_cast<BaseModule*>(ptr_M));
        modules.push_back(static_cast<BaseModule*>(spm_M));
    }

    ActRW Act_M;
    Nzf_M.connect(static_cast<BaseModule*>(&Act_M));

    
    int cycles = 80;
    for (int i = 0; i < cycles; i++) {
        if (i == 0) {
            Act_M.reg_addr_w = 0;
            *(reinterpret_cast<float*>(Act_M.acts_per_bank) + 0) = 0.0f;
            *(reinterpret_cast<float*>(Act_M.acts_per_bank) + 1) = 1.0f;
            *(reinterpret_cast<float*>(Act_M.acts_per_bank) + 2) = 1.0f;
            *(reinterpret_cast<float*>(Act_M.acts_per_bank) + 3) = 2.0f;
        }
        else if (i == 3) {
            Act_M.reg_addr_w = 1;
            *(reinterpret_cast<float*>(Act_M.acts_per_bank) + 0) = 1.0f;
            *(reinterpret_cast<float*>(Act_M.acts_per_bank) + 1) = 0.0f;
            *(reinterpret_cast<float*>(Act_M.acts_per_bank) + 2) = 0.0f;
            *(reinterpret_cast<float*>(Act_M.acts_per_bank) + 3) = 0.0f;
        }

        for (int m = 0; m < modules.size(); m++) {
            modules[m]->propagate();
        }

        std::cout << "Cycles:" << i << std::endl;
        std::cout << "==========================================="<<std::endl;

        for (int m = 0; m < modules.size(); m++) {
            modules[m]->update();
        }
    }
}

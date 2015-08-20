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

        Nzf_M.connect(static_cast<BaseModule*>(ptr_M));
        Nzf_M.connect(static_cast<BaseModule*>(spm_M));

        modules.push_back(static_cast<BaseModule*>(ptr_M));
        modules.push_back(static_cast<BaseModule*>(spm_M));
    }

    ActRW Act_M;
    Nzf_M.connect(static_cast<BaseModule*>(&Act_M));
    
    PtrRead *PtrR_M = static_cast<PtrRead*>(modules[1]);
    SpMatRead *SPMat_M = static_cast<SpMatRead*>(modules[2]);
    
    int cycles = 80;
    for (int i = 0; i < cycles; i++) {
        if (i == 0) {
            Act_M.reg_addr_w = 0;
            *(reinterpret_cast<float*>(Act_M.acts_per_bank) + 0) = 0.0f;
            *(reinterpret_cast<float*>(Act_M.acts_per_bank) + 1) = 1.0f;
            *(reinterpret_cast<float*>(Act_M.acts_per_bank) + 2) = 0.0f;
            *(reinterpret_cast<float*>(Act_M.acts_per_bank) + 3) = 2.0f;
        }
        else if (i == 3) {
            Act_M.reg_addr_w = 1;
            *(reinterpret_cast<float*>(Act_M.acts_per_bank) + 0) = 1.0f;
            *(reinterpret_cast<float*>(Act_M.acts_per_bank) + 1) = 0.0f;
            *(reinterpret_cast<float*>(Act_M.acts_per_bank) + 2) = 0.0f;
            *(reinterpret_cast<float*>(Act_M.acts_per_bank) + 3) = 0.0f;
        }
        else if (i == 6) {
            Act_M.reg_addr_w = 2;
            *(reinterpret_cast<float*>(Act_M.acts_per_bank) + 0) = 0.0f;
            *(reinterpret_cast<float*>(Act_M.acts_per_bank) + 1) = 0.0f;
            *(reinterpret_cast<float*>(Act_M.acts_per_bank) + 2) = 0.0f;
            *(reinterpret_cast<float*>(Act_M.acts_per_bank) + 3) = 0.0f;
        }
        else if (i == 7) {
            Act_M.reg_addr_w = 3;
            *(reinterpret_cast<float*>(Act_M.acts_per_bank) + 0) = 0.0f;
            *(reinterpret_cast<float*>(Act_M.acts_per_bank) + 1) = 3.0f;
            *(reinterpret_cast<float*>(Act_M.acts_per_bank) + 2) = 0.0f;
            *(reinterpret_cast<float*>(Act_M.acts_per_bank) + 3) = 0.0f;
        }

        for (int m = 0; m < modules.size(); m++) {
            modules[m]->propagate();
        }

        std::cout << "Cycles:" << i << std::endl;
        P_V(Nzf_M.pack_addr);
        P_V(Nzf_M.next_shift);
        P_V(Nzf_M.find);
        P_V(Nzf_M.index_buffer);
        P_V(Nzf_M.full[0]);
        P_V(Nzf_M.empty[0]);
        P_V(Nzf_M.act_index_output[0]);
        P_V(*(Nzf_M.valid_ptr[0]));
        P_V(*(Nzf_M.read_sp[0]));

        P_V(PtrR_M->valid);
        P_V(PtrR_M->start_addr);

        P_V(SPMat_M->valid);
        P_V(SPMat_M->memory_addr_shift);
        std::cout << "==========================================="<<std::endl;

        for (int m = 0; m < modules.size(); m++) {
            modules[m]->update();
        }
    }
}

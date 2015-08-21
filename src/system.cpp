
#include"module.h"
#include<vector>
#include<string>
using namespace std;

class System {
    System();
    ~System() {}
    void tic();

    vector<BaseModule*> modules;
    int num_modules;
    int cycles

}

#if DEBUG == 1
ActRW* act;
NzeroFetch* nzf;
PtrRead* ptr;
SpMatRead* spm;
ArithmUnit* aru;
#endif

BaseModule *ModuleCreate(ModuleType type, int num) {
    BaseModule *ans;
    switch (type) {
        case ActRW_k:
            ans = new ActRW[num];
            break;
        case NzeroFetch_k:
            ans = new NzeroFetch[num];
            break;
        case PtrRead_k:
            ans = new PtrRead[num];
            break;
        case SpMatRead_k:
            ans = new SpMatRead[num];
            break;
        case Arithm_k:
            ans = new ArithmUnit[num];
            break;
        default:
            LOG_ERROR("Unknown module type!");
    }
#if DEBUG == 1
    switch (type) {
        case ActRW_k:
            act = ans;
            break;
        case NzeroFetch_k:
            nzf = ans;
            break;
        case PtrRead_k:
            ptr= ans;
            break;
        case SpMatRead_k:
            spm = ans;
            break;
        case Arithm_k:
            aru = ans;
            break;
        default:
            LOG_ERROR("Unknown module type!");
    }
#endif
}

void Init(BaseModule *module) {
    ModuleType type = modules[i]->name();
    int id = modules[i]->id();
    string filename = datafile[type];
    filename += to_string(id) + ".dat";

    switch (type) {
        case ActRW_k:
            static_cast<ActRW*>(module)->init(filename.c_str());
            break;
        case NzeroFetch_k:
            break;
        case PtrRead_k:
            static_cast<PtrRead*>(module)->init(filename.c_str());
            break;
        case SpMatRead_k:
            static_cast<SpMatRead*>(module)->init(filename.c_str());
            break;
        case Arithm_k:
            break;
        default:
            LOG_ERROR("Unknown module type!");
    }
}
System::System() {
    num_modules = 0;
    cycles = 0;
    for (int i = 0; i < TYPES; i++) {
        num_modules += NumModules[i];
        BaseModule *base_ptr;
        base_ptr = ModuleCreate(i, NumModules[i]);
        for (int j = 0; j < NumModules[i]; j++) {
            modules.push_back(base_ptr + j);
        }
    }

    for (int i = 0; i < num_modules; i++) {
        Init(modules[i]);
        modules[i]->propagate();
    }

    for (int i = 0; i < num_modules; i++) {
        for (int j = 0; j < num_modules; j++) {
            if (topology[modules[i]->name()][modules[j]->name()] == Connect_by_id) {
                if (modules[i]->id() == modules[j]->name()) {
                    modules[i]->connect(modules[j])
                }
            }
            else if (topology[modules[i]->name()][modules[j]->name()] == Connect_all) {
                modules[i]->connect(modules[j])
            }
        }
    }
                
}

System::tic() {
    for (int i = 0; i < num_modules; i++) {
        modules[i]->propagate();
    }

    for (int i = 0; i < num_modules; i++) {
        modules[i]->update();
    }

    cycles++;
}


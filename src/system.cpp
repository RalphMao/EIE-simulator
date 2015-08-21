
#include"module.h"
#include"utils.h"
#include<vector>
#include<string>
#include<fstream>

using namespace std;

class System {
    public:
    System();
    ~System() {}
    void tic();

    inline bool done();
    void output(const char *output_file);

    vector<BaseModule*> modules;
    int num_modules;
    int cycles;

};

#if DEBUG == 1
vector<ActRW*> act;
vector<NzeroFetch*> nzf;
vector<PtrRead*> ptr;
vector<SpMatRead*> spm;
vector<ArithmUnit*> aru;
#endif

BaseModule *ModuleCreate(ModuleType type, int id) {
    BaseModule *ans = nullptr;
    switch (type) {
        case ActRW_k:
            ans = new ActRW;
            break;
        case NzeroFetch_k:
            ans = new NzeroFetch;
            break;
        case PtrRead_k:
            ans = new PtrRead(id);
            break;
        case SpMatRead_k:
            ans = new SpMatRead(id);
            break;
        case Arithm_k:
            ans = new ArithmUnit(id);
            break;
        default:
            LOG_ERROR("Unknown module type!");
    }
#if DEBUG == 1
    switch (type) {
        case ActRW_k:
            act.push_back(static_cast<ActRW*>(ans));
            break;
        case NzeroFetch_k:
            nzf.push_back(static_cast<NzeroFetch*>(ans));
            break;
        case PtrRead_k:
            ptr.push_back(static_cast<PtrRead*>(ans));
            break;
        case SpMatRead_k:
            spm.push_back(static_cast<SpMatRead*>(ans));
            break;
        case Arithm_k:
            aru.push_back(static_cast<ArithmUnit*>(ans));
            break;
        default:
            break;
    }
#endif
    return ans;
}

void Init(BaseModule *module) {
    ModuleType type = module->name();
    int id = module->id();
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
            static_cast<ArithmUnit*>(module)->init(filename.c_str());
            break;
        default:
            LOG_ERROR("Unknown module type!");
    }
}

System::System() {
    num_modules = 0;
    cycles = 0;

    // New all modules
    for (int i = 0; i < TYPES; i++) {
        num_modules += NumModules[i];
        BaseModule *base_ptr;
        for (int j = 0; j < NumModules[i]; j++) {
            base_ptr = ModuleCreate(static_cast<ModuleType>(i), j);
            modules.push_back(base_ptr);
        }
    }

    // Initialize all modules
    for (int i = 0; i < num_modules; i++) {
        Init(modules[i]);
        modules[i]->propagate();
    }

    // Connect necessary modules
    for (int i = 0; i < num_modules; i++) {
        for (int j = 0; j < num_modules; j++) {
            if (topology[modules[i]->name()][modules[j]->name()] == Connect_by_id) {
                if (modules[i]->id() == modules[j]->name()) {
                    modules[i]->connect(modules[j]);
                }
            }
            else if (topology[modules[i]->name()][modules[j]->name()] == Connect_all) {
                modules[i]->connect(modules[j]);
            }
        }
    }
                
}

void System::tic() {
    for (int i = 0; i < num_modules; i++) {
        modules[i]->propagate();
    }

    for (int i = 0; i < num_modules; i++) {
        modules[i]->update();
    }

    cycles++;
}

inline bool System::done() {
    return static_cast<ActRW*>(modules[0])->layer_complete;
}

void System::output(const char* output_file) {
    if (done()) {
        ActRW *ActRW_m = static_cast<ActRW*>(modules[0]);
        ofstream file(output_file, ios::out);
        if (file.is_open()) {
            for (int i = 0; i < ACTRW_maxcapacity; i++) {
                file << static_cast<float>(ActRW_m->ACTmem[1-ActRW_m->which][i]) << endl;
            }
            file.close();
        }
        else {
            LOG_ERROR("Unable to open the file!");
        }
    }
}

int main() {
    System system;
    LOG("System initialization done");
    while (!system.done()) {
        system.tic();
        LOG_DEBUG(("Cycle:"+(to_string(system.cycles))));
    }
    LOG("One layer done");
    system.output("output.txt");
    return 0;
}


#include"module.h"
#include"utils.h"
#include<vector>
#include<string>
#include<fstream>

using namespace std;

class System {
 public:
    System();
    ~System() {
    }
    void tic();
    void init();

    inline bool done();
    void output(const char *output_file);

    vector<BaseModule*> modules;
    int num_modules;
    int cycles;

};

#if DEBUG == 1
template class std::vector<ActRW*>;
template class std::vector<NzeroFetch*>;
template class std::vector<PtrRead*>;
template class std::vector<SpMatRead*>;
template class std::vector<ArithmUnit*>;
vector<ActRW*> act;
vector<NzeroFetch*> nzf;
vector<PtrRead*> ptr;
vector<SpMatRead*> spm;
vector<ArithmUnit*> aru;

void print_v() {
    P_V(act[0]->read_addr_reg);
    P_V(*(act[0]->write_enable_D[NUM_PE-1]));
    P_V(*(act[0]->write_addr_arithm_D[NUM_PE-1]));
    P_VS(act[0]->acts_per_bank, 4);

    P_V(nzf[0]->pack_addr);
    P_V(nzf[0]->write_enable);
    P_V(nzf[0]->index_buffer);
    P_V(nzf[0]->empty[1]);
    P_V(nzf[0]->act_index_output[1]);

    P_V(ptr[NUM_PE-1]->start_addr);
    P_V(ptr[NUM_PE-1]->end_addr);
    P_V(ptr[NUM_PE-1]->valid);

    P_V(spm[NUM_PE-1]->index);
    P_V(spm[NUM_PE-1]->code);

    P_V(aru[NUM_PE-1]->read_addr);
    P_V(*(reinterpret_cast<float*>(&aru[3]->result_muladd)));
}
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
    string filename = "data/";
    filename += datafile[type];
    if (type == PtrRead_k || type == SpMatRead_k) {
        filename += to_string(id);
    }
    filename += ".dat";

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

void System::init() {

    // Initialize all modules
    for (int i = 0; i < num_modules; i++) {
        Init(modules[i]);
        modules[i]->propagate();
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

    // Connect necessary modules
    for (int i = 0; i < num_modules; i++) {
        for (int j = 0; j < num_modules; j++) {
            if (topology[modules[i]->name()][modules[j]->name()] == Connect_by_id) {
                if (modules[i]->id() == modules[j]->id()) {
                    modules[i]->connect(modules[j]);
                }
            } else if (topology[modules[i]->name()][modules[j]->name()] == Connect_all) {
                modules[i]->connect(modules[j]);
            }
        }
    }

}

void System::tic() {
    for (int i = 0; i < num_modules; i++) {
        modules[i]->update();
    }

    for (int i = 0; i < num_modules; i++) {
        modules[i]->propagate();
    }

    cycles++;
}

inline bool System::done() {
    return static_cast<ActRW*>(modules[0])->layer_complete;
}

void System::output(const char* output_file) {
    ActRW *ActRW_m = static_cast<ActRW*>(modules[0]);
    ofstream file(output_file, ios::out | ios::binary);
    if (file.is_open()) {
        file.write(reinterpret_cast<char*>(ActRW_m->ACTmem[1 - ActRW_m->which]), ACTRW_maxcapacity * sizeof(float));
        file.close();
    } else {
        LOG_ERROR("Unable to open the file!");
    }
}

int main() {
    System system;
    ActRW *ControlUnit = static_cast<ActRW*>(system.modules[0]);
    ControlUnit->set_state(1, ACT_length, 0, 1);
    system.init();

    LOG("System initialization done");
    while (!system.done() || system.cycles < 9) {
        LOG_DEBUG(("Cycle:" + (to_string(system.cycles))));
#if DEBUG == 1
        if (system.cycles < 200) {
            print_v();
        }
#endif

        system.tic();
    }
    LOG("One layer done, cycles:" + to_string(system.cycles));
#if DEBUG == 1
    LOG("Efficiency:" + to_string(double(act[0]->valid_write_times) / system.cycles / NUM_PE));
    system.output("data/output.dat");
#endif
    return 0;
}


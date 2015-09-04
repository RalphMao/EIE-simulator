#ifndef COMPONENTS
#define COMPONENTS

#include<stdint.h>
#include<cstddef>
#if PROFILE == 1

class Register {
    public:
    Register(): data(0), changes(0) {}
    ~Register() {}
    inline Register& operator=(const uint32_t& x) {changes += data!= x; data = x; return *this;}
    inline operator uint32_t() {return data;}

    uint32_t data;
    int changes;
};

class Wire {
    public:
    Wire(): data(0), changes(0) {}
    ~Wire() {}
    inline uint32_t* operator&() {return &data;}
    inline Wire& operator=(const uint32_t& x) {changes += data!= x; data = x; return *this;}
    inline operator uint32_t() {return data;}

    uint32_t data;
    int changes;
};

class SharedWire {
    public:
    SharedWire(): ptr(NULL) {}
    inline SharedWire(Wire *ptr_w) {ptr = &(ptr_w->data);}
    inline SharedWire(uint32_t *ptr_w) {ptr = ptr_w;}
    ~SharedWire() {}
    inline SharedWire& operator=(Wire* ptr_w) {ptr = &(ptr_w->data); return *this;}
    inline uint32_t& operator*() {return *ptr;}

    uint32_t *ptr;
};
#else
typedef uint32_t Register;
typedef uint32_t Wire;
typedef const uint32_t* SharedWire;

#endif
typedef uint32_t* Memory;


#endif


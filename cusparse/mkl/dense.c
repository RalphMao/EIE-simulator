#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <time.h>
#include <mkl.h>
#include <mkl_cblas.h>
#include <mkl_blas.h>

#define Check(INFO) \
    cudaDeviceSynchronize(); \
    printf("Checkpoint:"#INFO", time: %ld ms\n", (clock()-start) * 1000 / CLOCKS_PER_SEC);\
    start = clock(); 

int main(int argc, char** argv) {
    clock_t start = clock();

    
    // Get weights ready
    float *weight, *weight_gpu;
    int m = atoi(argv[1]);
    int n = atoi(argv[2]);
    weight = (float*)malloc(m * n * sizeof(float));
    memset((void*)weight, 1, sizeof(float) * m * n);


    // Get activations ready
    float *act, *act_gpu;
    int m_v = n;
    int n_v = 4096;
    act = (float*)malloc(m_v * n_v * sizeof(float));
    memset((void*)act, 1, sizeof(float) * m_v * n_v);

    
    // Get extra things ready
    float *bias;
    bias = (float*)malloc(m * n_v * sizeof(float));
    memset((void*)bias, 0, sizeof(float) * m * n_v);

    float one = 1.0;
    float zero = 0.0;


    for (int time = 0; time < 1; time++) {
    for (int idx = 0; idx < n_v; idx++) {
        cblas_sgemv(CblasColMajor, CblasNoTrans, 
            m, n, one,
            weight, m,
            act+ m_v * idx, 1,
            zero, bias, 1);
    }
    }

    int batch_size = 64;
    for (int time = 0; time < 100; time++) {
    for (int idx = 0; idx < n_v / batch_size; idx ++) {
        cblas_sgemm(CblasColMajor, CblasNoTrans, CblasNoTrans,
            m, batch_size, n,
            one,
            weight, m,
            act+ idx * batch_size * m_v, m_v,
            zero, bias, m);
    }
    }

    return 0;
}


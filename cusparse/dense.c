#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <cuda_runtime.h> 
#include <cublas_v2.h>
#include <time.h>

#define Check(INFO) \
    cudaDeviceSynchronize(); \
    printf("Checkpoint:"#INFO", time: %ld ms\n", (clock()-start) * 1000 / CLOCKS_PER_SEC);\
    start = clock(); 

int main(int argc, char** argv) {
    clock_t start = clock();

    cublasHandle_t handle;
    cublasStatus_t status;
    status = cublasCreate(&handle);
    
    // Get weights ready
    float *weight, *weight_gpu;
    int m = 4096;
    int n = 4096;
    weight = (float*)malloc(m * n * sizeof(float));
    memset((void*)weight, 1, sizeof(float) * m * n);

    cudaMalloc((void**)&weight_gpu, m * n * sizeof(float));
    cudaMemcpy(weight_gpu, weight, m * n * sizeof(float), cudaMemcpyHostToDevice);
    

    // Get activations ready
    float *act, *act_gpu;
    int m_v = 4096;
    int n_v = 4096;
    act = (float*)malloc(m_v * n_v * sizeof(float));
    memset((void*)act, 1, sizeof(float) * m_v * n_v);

    cudaMalloc((void**)&act_gpu, m_v * n_v * sizeof(float));
    cudaMemcpy(act_gpu, act, m_v * n_v * sizeof(float), cudaMemcpyHostToDevice);
    
    // Get extra things ready
    float *bias;
    cudaMalloc((void**)&bias, m * n_v * sizeof(float));
    cudaMemset(bias, 0, m * n_v * sizeof(float));

    float one = 1.0;
    float zero = 0.0;

    Check(cublas_start)
    for (int time = 0; time < 1; time++) {
    for (int idx = 0; idx < n_v; idx++) {
        status = cublasSgemv(handle, CUBLAS_OP_N, 
            m, n, &one,
            weight_gpu, m,
            act_gpu + m_v * idx, 1,
            &zero, bias, 1);
        if (status != CUBLAS_STATUS_SUCCESS) { printf("%d,%dfailed", time, idx); return 1; } 
        }
    }
    Check(cublas_time_batchsize1)

    int batch_size = 64;
    for (int idx = 0; idx < n_v / batch_size; idx ++) {
        status = cublasSgemm(handle, CUBLAS_OP_N, CUBLAS_OP_N,
            m, batch_size, n,
            &one,
            weight_gpu, m,
            act_gpu + idx * batch_size * m_v, m_v,
            &zero, bias, m_v);
    }
    Check(cublas_time_batchsize)

    return 0;
}

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

#define Check_CUDA(INFO) \
    cudaEventRecord(stop_gpu_, 0); \
    cudaEventSynchronize(stop_gpu_); \
    cudaEventElapsedTime(&SECONDS, start_gpu_, stop_gpu_);\
    printf("CUDA Time Report-"#INFO": %.4f ms\n", SECONDS);\
    cudaEventRecord(start_gpu_, 0);

int main(int argc, char** argv) {
    clock_t start = clock();

    cudaSetDevice(1);
    cublasHandle_t handle;
    cublasStatus_t status;
    status = cublasCreate(&handle);


    
    // Get weights ready
    float *weight, *weight_gpu;
    float *weight2, *weight2_gpu;
    float *weight3, *weight3_gpu;
    int m = atoi(argv[1]);
    int n = 4096;
    int n2 = 1000;
    weight = (float*)malloc(m * n * sizeof(float));
    memset((void*)weight, 1, sizeof(float) * m * n);

    cudaMalloc((void**)&weight_gpu, m * n * sizeof(float));
    cudaMemcpy(weight_gpu, weight, m * n * sizeof(float), cudaMemcpyHostToDevice);
    
    weight2 = (float*)malloc( n * n * sizeof(float));
    memset((void*)weight2, 1, sizeof(float) * m * n);

    cudaMalloc((void**)&weight2_gpu, n * n * sizeof(float));
    cudaMemcpy(weight2_gpu, weight2, n * n * sizeof(float), cudaMemcpyHostToDevice);
    
    weight3 = (float*)malloc(n *n2* sizeof(float));
    memset((void*)weight3, 1, sizeof(float)  * n*n2);

    cudaMalloc((void**)&weight3_gpu, n2 * n * sizeof(float));
    cudaMemcpy(weight3_gpu, weight3, n2 * n * sizeof(float), cudaMemcpyHostToDevice);
    

    // Get activations ready
    float *act, *act_gpu;
    float *act2, *act2_gpu;
    float *act3, *act3_gpu;
    int m_v = n;
    int n_v = 4096;
    act = (float*)malloc(m_v * n_v * sizeof(float));
    memset((void*)act, 1, sizeof(float) * m_v * n_v);

    cudaMalloc((void**)&act_gpu, m_v * n_v * sizeof(float));
    cudaMemcpy(act_gpu, act, m_v * n_v * sizeof(float), cudaMemcpyHostToDevice);
    
    act2 = (float*)malloc(n * n_v * sizeof(float));
    memset((void*)act2, 1, sizeof(float) * n * n_v);

    cudaMalloc((void**)&act2_gpu, n * n_v * sizeof(float));
    cudaMemcpy(act2_gpu, act2, n * n_v * sizeof(float), cudaMemcpyHostToDevice);
    
    act3 = (float*)malloc(n * n_v * sizeof(float));
    memset((void*)act3, 1, sizeof(float) * n * n_v);

    cudaMalloc((void**)&act3_gpu, n * n_v * sizeof(float));
    cudaMemcpy(act3_gpu, act3, n * n_v * sizeof(float), cudaMemcpyHostToDevice);
    

    // Get extra things ready
    float *bias;
    cudaMalloc((void**)&bias, n2 * n_v * sizeof(float));
    cudaMemset(bias, 0, n2 * n_v * sizeof(float));

    float one = 1.0;
    float zero = 0.0;

    cudaEvent_t start_gpu_;
    cudaEvent_t stop_gpu_; 
    cudaEventCreate(&start_gpu_);
    cudaEventCreate(&stop_gpu_);
    float SECONDS;
    cudaEventRecord(start_gpu_, 0);

    for (int time = 0; time < 1; time++) {
    for (int idx = 0; idx < n_v; idx++) {
        status = cublasSgemv(handle, CUBLAS_OP_N, 
            m, n, &one,
            weight_gpu, m,
            act_gpu + m_v * idx, 1,
            &zero, act2_gpu, 1);
        if (status != CUBLAS_STATUS_SUCCESS) { printf("%d,%dfailed", time, idx); return 1; } 

        status = cublasSgemv(handle, CUBLAS_OP_N, 
            n, n, &one,
            weight2_gpu, n,
            act2_gpu + n * idx, 1,
            &zero, act3_gpu, 1);
        if (status != CUBLAS_STATUS_SUCCESS) { printf("%d,%dfailed", time, idx); return 1; } 

        status = cublasSgemv(handle, CUBLAS_OP_N, 
            n, n2, &one,
            weight3_gpu, n,
            act3_gpu + n2 * idx, 1,
            &zero, bias, 1);
        if (status != CUBLAS_STATUS_SUCCESS) { printf("%d,%dfailed", time, idx); return 1; } 
        }
    }
	Check_CUDA(cublas_time_batchsize1)

/*
    int batch_size = 64;
    for (int time = 0; time < 100; time++) {
    for (int idx = 0; idx < n_v / batch_size; idx ++) {
        status = cublasSgemm(handle, CUBLAS_OP_N, CUBLAS_OP_N,
            m, batch_size, n,
            &one,
            weight_gpu, m,
            act_gpu + idx * batch_size * m_v, m_v,
            &zero, bias, m);
    }
    }
    */
	Check_CUDA(cublas_time_batchsize64)

    return 0;
}


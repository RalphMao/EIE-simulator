#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <cuda_runtime.h> 
#include <cusparse.h>
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


void Init_from_file(char *filename, int *m, int *n, int *nnz, float **csr_val, int **csr_rowptr, int **csr_colind) {
    FILE *f = fopen(filename, "rb");
    fread(m, sizeof(int), 1, f);
    fread(n, sizeof(int), 1, f);
    fread(nnz, sizeof(int), 1, f);

    *csr_val = (float*)malloc(sizeof(float) * *nnz);
    *csr_rowptr = (int*)malloc(sizeof(int) * (*m + 1));
    *csr_colind = (int*)malloc(sizeof(int) * *nnz);

    fread(*csr_val, sizeof(float), *nnz, f);
    fread(*csr_rowptr, sizeof(int), (*m + 1), f);
    fread(*csr_colind, sizeof(int) , *nnz,f );

    fclose(f);
}

void Init_act_from_file(char* filename, int *m, int *n, float **act) {
    FILE *f = fopen(filename, "rb");
    fread(m, sizeof(int), 1, f);
    fread(n, sizeof(int), 1, f);

    *act = (float*)malloc(sizeof(float) * *m * *n);

    fread(*act, sizeof(float) , *m * *n, f);

    fclose(f);
}

int main(int argc, char** argv) {
    clock_t start = clock();

    cudaSetDevice(0);
    cusparseHandle_t handle=0;
    cusparseStatus_t status;
    status = cusparseCreate(&handle);
    cusparseMatDescr_t descr=0;

    int m,n, nnz;
    float *csr_val;
    int *csr_rowptr, *csr_colind; 
    float *csr_val_gpu;
    int *csr_rowptr_gpu, *csr_colind_gpu; 

    int m_v, n_v;
    float* act, *act_gpu;

    // Get Matrix ready
    Init_from_file(argv[1], &m, &n, &nnz, &csr_val, &csr_rowptr, &csr_colind);
    cudaMalloc((void**)&csr_val_gpu,nnz*sizeof(float));
    cudaMalloc((void**)&csr_colind_gpu,nnz*sizeof(int)); 
    cudaMalloc((void**)&csr_rowptr_gpu,(m+1)*sizeof(int)); 

    cudaMemcpy(csr_val_gpu, csr_val, (size_t)(nnz*sizeof(float)), cudaMemcpyHostToDevice);
    cudaMemcpy(csr_colind_gpu, csr_colind, (size_t)(nnz*sizeof(int)), cudaMemcpyHostToDevice);
    cudaMemcpy(csr_rowptr_gpu, csr_rowptr, (size_t)((m+1)*sizeof(int)), cudaMemcpyHostToDevice);

    status= cusparseCreateMatDescr(&descr); 
    if (status != CUSPARSE_STATUS_SUCCESS) { 
        return 1; 
    } 
    cusparseSetMatType(descr,CUSPARSE_MATRIX_TYPE_GENERAL); 
    cusparseSetMatIndexBase(descr,CUSPARSE_INDEX_BASE_ZERO); 

    // Get Activations ready

    m_v = n;
    n_v = 4096;
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

    cudaEvent_t start_gpu_;
    cudaEvent_t stop_gpu_; 
    cudaEventCreate(&start_gpu_);
    cudaEventCreate(&stop_gpu_);
    float SECONDS;

    cudaEventRecord(start_gpu_, 0);
    while (1) {
    for (int idx = 0; idx < n_v; idx++) {
        status = cusparseScsrmv(handle, CUSPARSE_OPERATION_NON_TRANSPOSE, m, n, nnz, &one, 
            descr, csr_val_gpu, csr_rowptr_gpu, csr_colind_gpu, act_gpu + idx * m_v, 
            &one, bias);
        if (status != CUSPARSE_STATUS_SUCCESS) { printf("%d,%dfailed", time, idx); return 1; } 
        }
    }
	Check_CUDA(cusparse_time_batchsize1)

    return 0;
}


#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <time.h>
#include <mkl_spblas.h>
#include <mkl.h>

#define Check(INFO) \
    printf("CheckpoMKL_INT:"#INFO", time: %ld ms\n", (clock()-start) * 1000 / CLOCKS_PER_SEC);\
    start = clock(); 


void Init_from_file(char *filename, MKL_INT *m, MKL_INT *n, MKL_INT *nnz, float **csr_val, MKL_INT **csr_rowptr, MKL_INT **csr_colind) {
    FILE *f = fopen(filename, "rb");
    fread(m, sizeof(MKL_INT), 1, f);
    fread(n, sizeof(MKL_INT), 1, f);
    fread(nnz, sizeof(MKL_INT), 1, f);

    *csr_val = (float*)malloc(sizeof(float) * *nnz);
    *csr_rowptr = (MKL_INT*)malloc(sizeof(MKL_INT) * (*m + 1));
    *csr_colind = (MKL_INT*)malloc(sizeof(MKL_INT) * *nnz);

    fread(*csr_val, sizeof(float), *nnz, f);
    fread(*csr_rowptr, sizeof(MKL_INT), (*m + 1), f);
    fread(*csr_colind, sizeof(MKL_INT) , *nnz,f );

    fclose(f);
}

void Init_act_from_file(char* filename, MKL_INT *m, MKL_INT *n, float **act) {
    FILE *f = fopen(filename, "rb");
    fread(m, sizeof(MKL_INT), 1, f);
    fread(n, sizeof(MKL_INT), 1, f);

    *act = (float*)malloc(sizeof(float) * *m * *n);

    fread(*act, sizeof(float) , *m * *n, f);

    fclose(f);
}

int main(int argc, char** argv) {
    clock_t start = clock();

    MKL_INT m,n, nnz;
    float *csr_val;
    MKL_INT *csr_rowptr, *csr_colind; 
    MKL_INT *csr_rowptre;

    MKL_INT m_v, n_v;
    float* act; 

    // Get Matrix ready
    Init_from_file(argv[1], &m, &n, &nnz, &csr_val, &csr_rowptr, &csr_colind);
    csr_rowptre = (MKL_INT*)malloc(m * sizeof(MKL_INT));
    for (MKL_INT i = 0; i < m; i++) {
        csr_rowptre[i] = csr_rowptr[i+1];
    }

    // Get Activations ready

    m_v = n;
    n_v = 4096;
    act = (float*)malloc(m_v * n_v * sizeof(float));
    memset((void*)act, 1, sizeof(float) * m_v * n_v);

    // Get extra things ready
    float *bias;
    bias = (float*)malloc(m * n_v * sizeof(float));
    memset((void*)bias, 1, sizeof(float) * m* n_v);

    float one = 1.0;
    float zero = 0.0;


	Check("start")
    MKL_INT batch_size = 64;
    for (MKL_INT time = 0; time < TIMES; time++) {
    for (MKL_INT idx = 0; idx < n_v / batch_size; idx ++) {
        mkl_scsrmm("N",
            &m, &batch_size, &n,
            &one, "GLNC",
            csr_val, csr_colind, 
            csr_rowptr, csr_rowptr + 1,
            act + idx * batch_size * m_v, &m_v,
            &zero, bias, &m);

    }}
	Check("sparse_mm")

    return 0;
}


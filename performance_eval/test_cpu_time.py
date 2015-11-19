import subprocess
import numpy as np
import scipy.sparse as sp
import time
dirs = ['alexnet_fc6',
'alexnet_fc7',
'alexnet_fc8', 
'vgg_fc6',   
'vgg_fc7',   
'vgg_fc8',  
'lenet_300_ip1',  
'lenet_300_ip2',  
'lenet_300_ip3',  
'lenet5_ip1',  
'lenet5_ip2',  
'neutalk_We',  
'neutalk_Wd',  
'neutalk_WLSTM']  
dirs = map(lambda x: 'data/' + x, dirs)

def load_fromfile(filename):
    f = open(filename)
    m, n, nnz = np.fromfile(f, 'int32', 3)
    data = np.fromfile(f, 'f', nnz)
    indptr = np.fromfile(f, 'int32', m+1)
    indices =np.fromfile(f, 'int32', nnz)

    mat = sp.csr_matrix((data, indices, indptr), shape=[m,n])
    return mat


def sparse_mul(matrix, vectors, batchsize = 1):
    n_v = vectors.shape[1]
    for i in range((n_v-1)/batchsize+1):
        res = matrix * vectors[:, batchsize * i:batchsize* i + batchsize]

def sparse_mul_sp(matrix, vectors):
    for vector in vectors:
        res = matrix * vector

def dense_mul(matrix, vectors, batchsize = 1):
    n_v = vectors.shape[1]
    res = np.zeros((matrix.shape[0], batchsize), dtype = vectors.dtype)
    for i in range((n_v-1)/batchsize+1):
        vector = vectors[:, batchsize * i:batchsize* i + batchsize]
        a = np.dot(matrix,vector)


f = open('log/log_cpu_time','w')
for dir_t in dirs:
    file_t = dir_t + '/matrix.dat'
    print file_t
    time_t = [0] * 6

    csr_mat = load_fromfile(file_t)
    mat = csr_mat.todense()
    vector_num = 256
    if 'neutalk' in dir_t:
        acts = np.ones((mat.shape[1], vector_num), dtype = 'f')
    else:
        acts = np.fromfile(dir_t + '/act.dat', dtype='f')
        n_v = acts.size / mat.shape[1]
        assert acts.size % mat.shape[1] == 0
        acts = np.repeat(acts, vector_num/n_v+1)
        acts = acts.reshape((mat.shape[1], acts.size/mat.shape[1]))
        acts = acts[:, :vector_num]

    spv = map(lambda x:sp.csr_matrix(acts[:,x:x+1]), range(acts.shape[1]))
    spm = map(lambda x:sp.csr_matrix(acts[:,64 * x:64*x + 64]), range(acts.shape[1]/64))

    print spm[0].shape
    print spv[0].shape
    print csr_mat.shape

    start_time = time.clock()
    dense_mul(mat, acts)
    time_t[0] = time.clock() - start_time

    start_time = time.clock()
    dense_mul(mat, acts, batchsize = 64)
    time_t[1] = time.clock() - start_time
    

    start_time = time.clock()
    sparse_mul(csr_mat, acts)
    time_t[2] = time.clock() - start_time

    start_time = time.clock()
    sparse_mul(csr_mat, acts, batchsize = 64)
    time_t[3] = time.clock() - start_time

    start_time = time.clock()
    sparse_mul_sp(csr_mat, spv)
    time_t[4] = time.clock() - start_time

    start_time = time.clock()
    sparse_mul_sp(csr_mat, spm)
    time_t[5] = time.clock() - start_time


    type = file_t.split('/')[-2]
    time_t = map(lambda x:x*1000*1000/vector_num, time_t)
    f.write('%s, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f\n'%((type,) +tuple(time_t)))
f.close()
